#include "brain.h"

// a strong connection should be a short distance, so distance = 1 / strength
vector<vector<double>> toDistance(Graph &g) {
    vector<vector<double>> d(g.n, vector<double>(g.n, INF)); // initialized to INF since that indicated no connection between 2 nodes
    for(int i = 0; i < g.n; i++) {
        for(int j = 0; j < g.n; j++) {
            if(i == j) d[i][j] = 0;
            else if(g.w[i][j] > 0) d[i][j] = 1.0 / g.w[i][j];
        }
    }
    return d;
}

// Floyd-Warshall
// after step k, d[i][j] is the shortest path from i to j that only goes through nodes 0 to k
// k has to be the outer loop because every step uses the answers of the step before
vector<vector<double>> floydWarshall(vector<vector<double>> d) {
    int n = d.size();
    for(int k = 0; k < n; k++) {
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < n; j++) {
                if(d[i][k] + d[k][j] < d[i][j]) d[i][j] = d[i][k] + d[k][j];
            }
        }
    }
    return d;
}

// global efficiency = average of (1 / distance over all pairs)
double globalEfficiency(vector<vector<double>> &d) {
    int n = d.size();
    if(n < 2) return 0;
    double sum = 0;
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            if(i != j && d[i][j] < INF) sum += 1.0 / d[i][j];
        }
    }
    return sum / (n * (n - 1.0));
}
