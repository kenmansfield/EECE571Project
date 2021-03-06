#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/LoopPass.h"
#include <llvm/Analysis/LoopInfo.h>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/PredIteratorCache.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"

#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <unistd.h>

using namespace std;
using namespace llvm;

namespace {

bool ReadInputParameters();
void outputToFile();

bool SHOW_DEBUG_INFO = true;

	// Unlike our Assignment code, we use a LoopPass instead of a function pass.
struct CodePerforationPass : public LoopPass 
{
	static char ID;			// Pass identification
	int mCurrentLoop;
	
	CodePerforationPass() : LoopPass(ID), mCurrentLoop(-1) 
	{
		ReadInputParameters();
	}
	
	~CodePerforationPass()
	{
		outputToFile();
	}
	
	LoopInfo *LI;

public:

	bool runOnLoop(Loop *loop, LPPassManager &LPM) override;
	bool attemptToPerorateLoop(Loop *loop);
	void perforateLoop(Loop *loop);

	virtual void getAnalysisUsage(AnalysisUsage &AU) const override
	{
		AU.addRequired<LoopInfoWrapperPass>();
		AU.addRequiredID(LoopSimplifyID);
		AU.addPreservedID(LoopSimplifyID);
	}
	using llvm::Pass::doFinalization;

	bool doFinalization() override 
	{
		return false;
	}
};

char CodePerforationPass::ID = 0;
static RegisterPass<CodePerforationPass> X("CodePerforationPass",
			    "Perforating Loops for EECE571", false, true);


string sOutputFile = "outputParameters.txt";
vector< vector<int> > vLoopInfo;
vector< vector<int> > vParameterFile;
//string sFileName = "~/University/EECE571p/Project/pass/inputParameters.txt";
string sFileName = "inputParameters.txt";
bool ReadInputParameters()
{
	if(SHOW_DEBUG_INFO)
	{
		char *path = NULL;
		path = getcwd(NULL, 0); // or _getcwd
		if ( path != NULL)
	   		printf("%s\n", path);
	   	
	   	cout << path << endl;
	}
   	
	string fileName = sFileName;
    ifstream infile;
    infile.open(fileName.c_str());
    std::string line;
    if(!infile.is_open())
    {	
    	cout << "missing input file!\n";
        return false;
    }

    int i = 0;
    while( getline(infile, line) )
    {
    	vector<int> tempVec;
        istringstream lineStream(line);
        int x;
        while(lineStream >> x)
        {
        	tempVec.push_back(x);
        }
        vParameterFile.push_back(tempVec);
        i++;
    }
    return true;
}

void outputToFile()
{
	if(SHOW_DEBUG_INFO) { cout << "writing to: " << sOutputFile << endl; }
	
    std::ofstream outFile(sOutputFile);
    
    if (!outFile.is_open())
    {
        cout << "error opening file";
        return;
    }
    
    //cout << "vLoopInfo size: " << vLoopInfo.size();
    for(auto v : vLoopInfo)
    {
    	//cout << v[0] << " " << v[1] << endl;
    	//cout << "v size: " << v.size();
    	outFile << v[0] << " " << v[1] << endl;
    }
    outFile.close();
}

bool CodePerforationPass::runOnLoop(Loop *loop, LPPassManager &LPM) 
{
	mCurrentLoop++;
	std::vector<int> newLoop = {mCurrentLoop,-1};
	vLoopInfo.push_back(newLoop);
	
	if(SHOW_DEBUG_INFO) {cout << "Loop #" << mCurrentLoop << endl;}
	
	LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
	return attemptToPerorateLoop(loop);
}

// Make an attempt to perforate this loop. We do this by determining if this
// loop is indeed perforatable. If it is, pass it on to the perforator.
// If it is not, then return false.
bool CodePerforationPass::attemptToPerorateLoop(Loop *loop) 
{
	//start by looking at the loop header.
	Instruction *lHead = loop->getHeader()->begin();
	
	// Not sure what this is for.
	//std::stringstream logStream;
	//logStream << "loop at " << lHead->getDebugLoc();
	//std::string identifier = logStream.str();
	
	// Get the header, and the function that this header belongs to.
	if(loop->getHeader() == false)
	{
		// This loop has no header, cannot continue!
		cout << "loop->getHeader() is false!\n";
		//return false;
	}
	
	if(loop->getLoopPreheader() == false)
	{
		// This loop has no header, cannot continue!
		cout << "loop->getHeader() is false!\n";
		return false;
	}
	
	if(loop->getLoopLatch() == false)
	{
		cout << "getLoopLatch() is false!\n";
		return false;
	}
	
	//cout << "Ok to perforate!\n";
	
	Instruction *headerInstruction = loop->getHeader()->begin();
	Function *func = headerInstruction->getParent()->getParent();
	std::string funcName = func->getName().str();
	
	perforateLoop(loop);
		      
  
      return false;
    }

  
   void CodePerforationPass::perforateLoop(Loop *loop) 
    {
      // Next, make sure the header (condition block) ends with a body/exit
      // conditional branch.
		BranchInst *condBranch = dyn_cast<BranchInst>(loop->getHeader()->getTerminator());
		if (condBranch == false || condBranch->getNumSuccessors() != 2) 
		{
			//cout << "no loop condition\n";
			return;
		}
      
		BasicBlock *bodyBlock;
		if (condBranch->getSuccessor(0) == loop->getExitBlock()) 
		{
			bodyBlock = condBranch->getSuccessor(1);
		} 
		else if (condBranch->getSuccessor(1) == loop->getExitBlock()) 
		{
			bodyBlock = condBranch->getSuccessor(0);
		} 
		else 
		{
			//cout << "No exit to the loop condition\n";
			return;
		}
		
		// Set this loop as available for perforation!
		vLoopInfo[mCurrentLoop][1] = 1;
		
		if(vParameterFile[0][1] == -2)
		{
			// If the perforation rate is set to -2, this is a dry run, to get the loop info!
			// do not perforate.
			//cout << "not perforating this time around\n";
			return;
		}
		

		int perforationRate = vParameterFile[mCurrentLoop][1];
		
		if(perforationRate == 1 || perforationRate == -1)
		{
			// 1 means nothing to do
			// -1 means this loop is not perforatable.
			return;
		}
		
		if(SHOW_DEBUG_INFO) { cout << "Perforating this loop with value: " << vParameterFile[mCurrentLoop][1] << endl;}
		
		
		
		BasicBlock *Header = loop->getLoopLatch(); //loop->getHeader();
  
		// Loop over all of the PHI nodes in the basic block, calculating the
		// induction variables that they represent... stuffing the induction variable
		// info into a vector...
		//
		/*std::vector<InductionVariable> IndVars;    // Induction variables for block
		BasicBlock::iterator AfterPHIIt = Header->begin();
		for (; PHINode *PN = dyn_cast<PHINode>(AfterPHIIt); ++AfterPHIIt)
		{
			IndVars.push_back(InductionVariable(PN, Loops));
		}*/
		/*
		
		BasicBlock *latch = loop->getLoopLatch();
		//std::vector<InductionVariable> IndVars;    // Induction variables for block
		BasicBlock::iterator bbIt = latch->begin();
		
		for (; Instruction *I = dyn_cast<Instruction>(bbIt); ++bbIt)
		{
			IndVars.push_back(InductionVariable(PN, Loops));
			

		}*/
		
		
		for (BasicBlock::iterator I = Header->begin(), E = --Header->end(); I != E; ++I)
		{
			if (Instruction *inst = dyn_cast<Instruction >(I))  
			{
				const unsigned OpCode = inst->getOpcode();
				//cout << "iter\n";
				if (OpCode && OpCode == Instruction::Add)
				{
					//cout << "add inst\n";
					//ConstantInt *C = dyn_cast<ConstantInt>(inst->getOperand(1));
					
					Value *oldValue = inst->getOperand(1);
					
					if(oldValue)
					{
					   /* construct new constant; */
					   Value *newvalue = ConstantInt::get(oldValue->getType(), perforationRate); 

					   /* replace operand with new value */
					   inst->setOperand(1, newvalue);
					}
					
					//llvm::ConstantInt::get(context, llvm::APInt(/*nbits*/32, value, /*bool*/is_signed));
					
					//inst->setOperand(1, 2);

				}
			}
		}
		
    }    
    /*
    void CodePerforationPass::perforateLoop(Loop *loop, int perforationRate) 
    {
      // Next, make sure the header (condition block) ends with a body/exit
      // conditional branch.
		BranchInst *condBranch = dyn_cast<BranchInst>(loop->getHeader()->getTerminator());
		if (condBranch == false || condBranch->getNumSuccessors() != 2) 
		{
			//errs() << "malformed loop condition\n";
			cout << "no loop condition\n";
			return;
		}
      
		BasicBlock *bodyBlock;
		if (condBranch->getSuccessor(0) == loop->getExitBlock()) 
		{
			bodyBlock = condBranch->getSuccessor(1);
		} 
		else if (condBranch->getSuccessor(1) == loop->getExitBlock()) 
		{
			bodyBlock = condBranch->getSuccessor(0);
		} 
		else 
		{
			cout << "No exit to the loop condition\n";
			return;
		}

		//Change the context.
		Module *theModule = loop->getHeader()->getParent()->getParent();
		IRBuilder<> builder(theModule->getContext());
		Value *result;

      // Allocate stack space for the counter.
      // LLVM "alloca" instructions go in the function's entry block. Otherwise,
      // they have to adjust the frame size dynamically (and, in my experience,
      // can actually segfault!). And we only want one of these per static loop
      // anyway.
     	builder.SetInsertPoint(
          loop->getLoopPreheader()->getParent()->getEntryBlock().begin());

	    DataLayout layout(theModule->getDataLayout());
		IntegerType *iType = Type::getIntNTy(theModule->getContext(), layout.getPointerSizeInBits());
						  
		AllocaInst *counterAlloca = builder.CreateAlloca(
			iType,
			0,
			"new_counter"
		);
		
		// Initialize the counter in the preheader.
		builder.SetInsertPoint(loop->getLoopPreheader()->getTerminator());
		builder.CreateStore(
			ConstantInt::get(iType, 0, false),
			counterAlloca
		);

      // Increment the counter in the latch.
      builder.SetInsertPoint(loop->getLoopLatch()->getTerminator());
      result = builder.CreateLoad(counterAlloca, 
      "kmproj_temp"
      );
      result = builder.CreateAdd(
          result,
          ConstantInt::get(iType, 1, false),
          "kmproj_increment"
      );
      builder.CreateStore(
          result,
          counterAlloca
      );

      // Check the counter before the loop's body.
      BasicBlock *checkBlock = BasicBlock::Create(
          theModule->getContext(),
          "kmproj_cond",
          bodyBlock->getParent(),
          bodyBlock
      );
      
      builder.SetInsertPoint(checkBlock);
      result = builder.CreateLoad(
          counterAlloca,
          "kmproj_tmp"
      );
      
      // Check whether the low n bits of the counter are zero.
      result = builder.CreateTrunc(
          result,
          Type::getIntNTy(theModule->getContext(), 2),
          "kmproj_trunc"
      );
      
      result = builder.CreateIsNull(
          result,
          "kmproj_cmp"
      );
      
      // Not sure about this.
      BasicBlock *skipDest = loop->getLoopLatch();
      result = builder.CreateCondBr(
          result,
          bodyBlock,
          skipDest
      );

      // Change the condition block to point to our new condition
      // instead of the body.
      condBranch->setSuccessor(0, checkBlock);

      // Add condition block to the loop structure.
      loop->addBasicBlockToLoop(checkBlock, *LI);
    }
    */
}
