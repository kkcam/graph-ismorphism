# Constructing Difficult Examples of Graph Isomorphism [CDEGI] Source Code
This package provides logic in generating graphs from xor systems to be executed on Traces. It is essentially a three part system:
1. Produce a random graph as decribed in the paper.
2. Check for 'unique satisfiability' using cryptominisat.
3. Check the graph for automorphisms using nauty.
 
## Requirements
* Linux OS
* Python 2.7 and various packages such as matplotlib and numpy. 

## Installation 
CDEGI requires two main external components. One is a SAT Solver (cryptominisat) and the other is a GI Solver (nauty).
* Installation of Cryptominisat: https://github.com/msoos/cryptominisat.git
* Installation of Nauty: http://pallini.di.uniroma1.it/

This project requires the cryptominisat program to be available in the folder "/assets/sat/cryptominisat/build/cryptominisat5". (The gauss off build must be in the "/assets/sat/cryptominisat/build_gauss/cryptominisat5" folder). An untested built version is provided with the repo, which can be replaced.

Nauty can be installed using 'sudo apt install nauty'. However, the program actually used is 'dreadnaut', which must be installed as a executable on the OS (i.e. we should be able to run 'dreadnaut' from the terminal).

Various other components, such as installing the plotting library and python libraries are not mentioned here.

 
 ## How to use
 python main.py
 
 ### Decriptions
 * sat.py houses the logic for testing and generating uniquly satisfiable graphs using cryptominisat.
 * gi.py houses the logic for testing for automorphisms using dreadnaut aka nauty.
 * main.py binds the logic of the previous files - tests are provided.

### Prebuilt Graphs For Dreadnaut
Graphs varying in complexity, generated using different parameters, are provided within the assets/construction_packages folder. These graphs are in the .dre format and are compatible with dreadnaut.

## Program usages
Conauto: graph-isomorphism/assets/programs/conauto-2.03/bin/conauto -aut -dv {dimacsGraph}

Bliss: graph-isomorphism/assets/programs/bliss-0.73/bliss {dimacsGraph}

Traces: dreadnaut

\> At -a V=0 -m <"{dreGraph.dre}" x q

Sparse Nauty: dreadnaut

\> As -a V=0 -m <"{dreGraph.dre}" x q
    
