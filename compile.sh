g++ -c -std=c++11 -Wall -o Search.o Search.cpp
g++ -c -std=c++11 -Wall -o Evaluate.o Evaluate.cpp
g++ -c -std=c++11 -g -Wall -o Simulate.o Simulate.cpp
g++ -c -std=c++11 -Wall -o sDrunk.o sDrunk.cpp

g++ -std=c++11 -Wall -o sDrunk Search.o Simulate.o Evaluate.o sDrunk.o
