# g++ -c -Wall -std=c++11 -o sDrunk.o sDrunk.cpp
# g++ -MM -Wall -std=c++11 sDrunk.cpp > sDrunk.d
# -include sDrunk:.o=.d
# g++ -std=c++11 -g -Wall sDrunk.o

g++ -c -Wall -o Serch.o Serch.cpp
G++ -c -Wall -o Evalute.o Evalute.cpp
g++ -std=c++11 -g -Wall -o sDrunk sDrunk.cpp
