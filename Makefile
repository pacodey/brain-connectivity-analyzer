# mingw32-make       builds brain
# mingw32-make run   runs it on the eyes open and eyes closed recordings

brain: main.cpp edf.cpp signal.cpp paths.cpp mst.cpp visualize.cpp brain.h
	g++ -std=c++11 -O2 -Wall main.cpp edf.cpp signal.cpp paths.cpp mst.cpp visualize.cpp -o brain

run: brain
	./brain data/S001R01.edf eyes_open
	./brain data/S001R02.edf eyes_closed
