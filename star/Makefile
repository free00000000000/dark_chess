CXX = g++ -std=c++11 -O3 -g -static

CLEAN = rm -rf

all:
	$(CXX) *.cpp -c
	$(CXX) star0_main.o star0.o -o star0
	$(CXX) star1_main.o star1.o -o star1
	$(CLEAN) *.o

