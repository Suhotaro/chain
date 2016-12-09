all: chain
	
chain: main.cpp
	g++ -std=c++11 main.cpp -o main -lpthread
