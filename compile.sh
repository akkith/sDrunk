g++ -c -std=c++11 -Wall -o Search.o Search.cpp
g++ -c -std=c++11 -Wall -o Evaluate.o Evaluate.cpp
#g++ -std=c++11 -g -Wall -o sDrunk sDrunk.cpp
g++ -c -std=c++11 -Wall -o sDrunk.o sDrunk.cpp

gcc -std=c++11 -Wall -o sDrunk Search.o Evaluate.o sDrunk.o
