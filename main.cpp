#include "brain.h"

// prints a node like "F3 (BA 9, Frontal)"
string describe(Graph &g, int i) {
    if(g.ba[i] == -1) return g.names[i];
    return g.names[i] + " (BA " + to_string(g.ba[i]) + ", " + g.area[i] + ")";
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        cout << "usage: brain <file.edf> [name] [reject limit in uV]" << endl;
        cout << "example: brain data/S001R01.edf eyes_open 400" << endl;
        return 0;
    }
    string file = argv[1];
    string name = "output";
    double reject = 400;
    if(argc >= 3) name = argv[2];
    if(argc >= 4) reject = stod(argv[3]);

    // 1. read the eeg and build the graph (alpha band, 8 to 13 Hz)
    Recording rec;
    if(!readEdf(file, rec)) return 1;
    Graph g = coherency(rec, 8, 13, reject);
    readBrodmann("brodmann_map.csv", g);

    // 2. mst: kruskal and prim should give the same total
    vector<Edge> tree = kruskal(g);
    vector<Edge> tree2 = prim(g);
    double total = 0, total2 = 0;
    for(int k = 0; k < tree.size(); k++) total += tree[k].w;
    for(int k = 0; k < tree2.size(); k++) total2 += tree2[k].w;
    vector<int> deg = degrees(g.n, tree);
    int opposite = 0;
    for(int k = 0; k < tree.size(); k++) {
        if(cos(g.phase[tree[k].u][tree[k].v]) < 0) opposite++;
    }

    cout << endl << "mst: " << tree.size() << " edges" << endl;
    cout << "total strength: kruskal " << total << ", prim " << total2 << endl;
    cout << "opposite phase edges: " << opposite << endl;

    // top 5 hubs
    cout << "hubs:" << endl;
    vector<bool> used(g.n, false);
    for(int h = 0; h < 5 && h < g.n; h++) {
        int best = -1;
        for(int i = 0; i < g.n; i++) {
            if(!used[i] && (best == -1 || deg[i] > deg[best])) best = i;
        }
        used[best] = true;
        cout << "  " << describe(g, best) << " degree " << deg[best] << endl;
    }

    // shortest paths
    vector<vector<double>> direct = toDistance(g);
    vector<vector<double>> dist = floydWarshall(direct);
    cout << endl << "global efficiency: " << globalEfficiency(dist) << endl;

    // visualization
    string picture = "head_" + name + ".svg";
    drawHead(picture, g, tree, deg);
    cout << endl << "saved " << picture << endl;
    return 0;
}
