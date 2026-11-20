#include "brain.h"

// we want the mst with the strongest edges, so these build a maximum spanning tree on the strengths
// this gives the same tree as a minimum spanning tree on the distances 1 / w

vector<int> parent;

int findRoot(int x) {
    if(parent[x] != x) parent[x] = findRoot(parent[x]); // path compression
    return parent[x];
}

bool unite(int a, int b) {
    a = findRoot(a);
    b = findRoot(b);
    if(a == b) return false; // already connected, this edge would make a cycle
    parent[b] = a;
    return true;
}

bool stronger(Edge a, Edge b) {
    return a.w > b.w;
}

// kruskal: 
// sort the edges from strongest to weakest take each edge unless it makes a cycle
vector<Edge> kruskal(Graph &g) {
    vector<Edge> edges;
    for(int i = 0; i < g.n; i++) {
        for(int j = i + 1; j < g.n; j++) {
            if(g.w[i][j] > 0) {
                Edge e;
                e.u = i;
                e.v = j;
                e.w = g.w[i][j];
                edges.push_back(e);
            }
        }
    }
    sort(edges.begin(), edges.end(), stronger);

    parent.resize(g.n);
    for(int i = 0; i < g.n; i++) parent[i] = i;

    vector<Edge> tree;
    for(int k = 0; k < edges.size(); k++) {
        if(unite(edges[k].u, edges[k].v)) tree.push_back(edges[k]);
    }
    return tree;
}

// prim: 
// start from node 0 and keep adding the strongest edge that leaves the tree
vector<Edge> prim(Graph &g) {
    vector<bool> inTree(g.n, false);
    vector<double> best(g.n, -1); // strongest edge from the tree to each node
    vector<int> from(g.n, -1);
    vector<Edge> tree;

    for(int step = 0; step < g.n; step++) {
        int u = -1;
        for(int v = 0; v < g.n; v++) {
            if(!inTree[v] && (u == -1 || best[v] > best[u])) u = v;
        }
        inTree[u] = true;
        if(from[u] != -1) {
            Edge e;
            e.u = from[u];
            e.v = u;
            e.w = g.w[from[u]][u];
            tree.push_back(e);
        }
        for(int v = 0; v < g.n; v++) {
            if(!inTree[v] && g.w[u][v] > best[v]) {
                best[v] = g.w[u][v];
                from[v] = u;
            }
        }
    }
    return tree;
}

// number of tree edges that touch each node
vector<int> degrees(int n, vector<Edge> &tree) {
    vector<int> deg(n, 0);
    for(int k = 0; k < tree.size(); k++) {
        deg[tree[k].u]++;
        deg[tree[k].v]++;
    }
    return deg;
}
