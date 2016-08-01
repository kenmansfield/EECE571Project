##Code Perforation with LLVM
###Final Project
###EECE 571p: Program Analysis and Optimization

**Abstract** — Approximate computing describes a class of algorithms that are designed to tolerate some loss of quality or
accuracy in return for increased performance. Modern computations, such as multimedia encoding and machine
learning, can be adjusted to meet the requirements of the user. Such algorithms generally require domain-specific
techniques developed specifically to fine tune the computation’s performance.
Loop perforation is a technique for improving the performance of approximate computations by reducing the number of
iterations executed. An optimization algorithm identifies tunable loops that respond well to perforation and automatically
generates variants of approximate computations. The result is a set of ideal perforation rates for each of the tunable
loops that return significant speedups while maintaining sufficient accuracy. Perforation is performed by an LLVM
compiler pass which modifies the method in which the induction variable is incremented in loops such that entire
iterations are skipped. The implementation is evaluated by applying code perforation to applications contained within the
PARSEC benchmark suite. Results show that significant speedups in code execution time are achievable while
maintaining reasonable accuracy.

> Index Terms—C++, Python, Compilers, Approximate Computing, LLVM.

[Final Report PDF](Ken Mansfield - EECE571p - Final Report.pdf)

This project is split into two repositories, this repository is the C++ LLVM pass. 

[This repository is the Python automation script plus test benchmarks](https://github.com/kenmansfield/EECE571Project-Pass)
