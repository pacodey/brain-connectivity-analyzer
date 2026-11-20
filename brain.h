#ifndef BRAIN_H // to protect the header file from being compiled more than once
#define BRAIN_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
using namespace std;

const double INF = 1e18;
const double PI = 3.14159265358979;

// one eeg recording discretizedd
struct Recording {
    double fs; // samples per second
    vector<string> names; // electrode names (C3, CP2)
    vector<vector<double>> data; // data[channel][sample] in microvolts
};

// the brain network
struct Graph {
    int n;
    vector<string> names;
    vector<int> ba; // brodmann area which electrode falls under
    vector<string> area; // frontal, visual
    vector<double> x, y; // position on the head picture
    vector<vector<double>> w; // edge strength from 0 to 1
    vector<vector<double>> phase; // edge timing in radians, 0 = together, pi = opposite
};

struct Edge {
    int u, v; // electrode 1, electrode 2
    double w; // weight
};

// edf.cpp
bool readEdf(string path, Recording &rec);
void readBrodmann(string path, Graph &g);

// signal.cpp
void fft(vector<complex<double>> &a, bool inverse);
Graph coherency(Recording rec, double lo, double hi, double rejectUV);

// paths.cpp
vector<vector<double>> toDistance(Graph &g);
vector<vector<double>> floydWarshall(vector<vector<double>> d);
double globalEfficiency(vector<vector<double>> &d);

// mst.cpp
vector<Edge> kruskal(Graph &g);
vector<Edge> prim(Graph &g);
vector<int> degrees(int n, vector<Edge> &tree);

// visualize.cpp
void drawHead(string path, Graph &g, vector<Edge> &tree, vector<int> &deg);

#endif
