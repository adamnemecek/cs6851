ppmap:	ppmap.cpp
	clang++ -std=c++11 -Wall -O2 -o ppmap ppmap.cpp

clean:
	rm -f ppmap
