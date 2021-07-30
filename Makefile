CXX = g++ -std=c++11 -O3 -g -static

CLEAN = rm -rf

all:
	$(CXX) src/*.cpp -c
	$(CXX) main.o MCTS.o -o MCTS
	$(CLEAN) *.o

