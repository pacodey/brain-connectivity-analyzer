#include "brain.h"

// draws the head from above with the mst on it
// red line = in step, blue line = opposite, thicker = stronger, bigger dot = hub
void drawHead(string path, Graph &g, vector<Edge> &tree, vector<int> &deg) {
    double cx = 320, cy = 330, r = 220;
    ofstream f(path.c_str());
    f << "<svg xmlns='http://www.w3.org/2000/svg' width='640' height='640' font-family='sans-serif'>\n";
    f << "<rect width='100%' height='100%' fill='white'/>\n";
    f << "<circle cx='" << cx << "' cy='" << cy << "' r='" << r << "' fill='none' stroke='gray' stroke-width='2'/>\n";
    // nose
    f << "<polygon points='" << cx - 18 << "," << cy - r << " " << cx << "," << cy - r - 25 << " "
      << cx + 18 << "," << cy - r << "' fill='none' stroke='gray' stroke-width='2'/>\n";

    for(int k = 0; k < (int)tree.size(); k++) {
        int u = tree[k].u, v = tree[k].v;
        if(g.x[u] == INF || g.x[v] == INF) continue; // no position known
        string colour = "red";
        if(cos(g.phase[u][v]) < 0) colour = "blue";
        f << "<line x1='" << cx + g.x[u] * r << "' y1='" << cy - g.y[u] * r
          << "' x2='" << cx + g.x[v] * r << "' y2='" << cy - g.y[v] * r
          << "' stroke='" << colour << "' stroke-width='" << 1 + 5 * tree[k].w << "'/>\n";
    }
    for(int i = 0; i < g.n; i++) {
        if(g.x[i] == INF) continue;
        double px = cx + g.x[i] * r, py = cy - g.y[i] * r;
        double dot = 4 + 1.5 * min(deg[i], 8);
        f << "<circle cx='" << px << "' cy='" << py << "' r='" << dot << "' fill='white' stroke='black'/>\n";
        f << "<text x='" << px << "' y='" << py - dot - 3 << "' font-size='10' text-anchor='middle'>"
          << g.names[i] << "</text>\n";
    }
    f << "<text x='20' y='620' font-size='12'>red = in step, blue = opposite, "
      << "thicker = stronger, bigger dot = hub</text>\n";
    f << "</svg>\n";
}
