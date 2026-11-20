#include "brain.h"

// edf file layout
// bytes 0 to 255: general info as text
// then 256 bytes per channel: all the labels first, then all the units, and so on then the samples as 16 bit numbers, stored in "records"
bool readEdf(string path, Recording &rec) {
    ifstream file(path.c_str(), ios::binary);
    if(!file) {
        cout << "cannot open " << path << endl;
        return false;
    }
    stringstream whole;
    whole << file.rdbuf();
    string b = whole.str(); // the whole file

    int headerBytes = stoi(b.substr(184, 8));
    int records = stoi(b.substr(236, 8));
    double recordSec = stod(b.substr(244, 8));
    int ns = stoi(b.substr(252, 4)); // number of channels

    vector<string> label(ns);
    vector<double> physMin(ns), physMax(ns), digMin(ns), digMax(ns);
    vector<int> perRecord(ns);
    int p = 256;

    // labels are like "Fc5.            ", so we need to turn them into "FC5" to match brodmann_map.csv
    for(int i = 0; i < ns; i++) {
        for(int k = 0; k < 16; k++) {
            char ch = b[p + 16 * i + k];
            if(ch != '.' && ch != ' ') label[i] += (char)toupper(ch);
        }
    }

    p += 16 * ns;
    p += 80 * ns; // skip transducer type
    p += 8 * ns; // skip units
    for(int i = 0; i < ns; i++) physMin[i] = stod(b.substr(p + 8 * i, 8));
    p += 8 * ns;
    for(int i = 0; i < ns; i++) physMax[i] = stod(b.substr(p + 8 * i, 8));
    p += 8 * ns;
    for(int i = 0; i < ns; i++) digMin[i] = stod(b.substr(p + 8 * i, 8));
    p += 8 * ns;
    for(int i = 0; i < ns; i++) digMax[i] = stod(b.substr(p + 8 * i, 8));
    p += 8 * ns;
    p += 80 * ns; // skip prefiltering
    for(int i = 0; i < ns; i++) perRecord[i] = stoi(b.substr(p + 8 * i, 8));

    // keep the eeg channels, skip the annotations channel
    vector<int> keep(ns, -1);
    for(int i = 0; i < ns; i++) {
        if(label[i].find("ANNOTATION") != string::npos) continue;
        keep[i] = rec.names.size();
        rec.names.push_back(label[i]);
        rec.fs = perRecord[i] / recordSec;
    }
    rec.data.resize(rec.names.size());

    int pos = headerBytes;
    for(int r = 0; r < records; r++) {
        for(int i = 0; i < ns; i++) {
            double scale = (physMax[i] - physMin[i]) / (digMax[i] - digMin[i]);
            for(int s = 0; s < perRecord[i]; s++) {
                short value = (short)((unsigned char)b[pos] | ((unsigned char)b[pos + 1] << 8)); // low byte comes first
                pos += 2;
                if(keep[i] != -1) {
                    rec.data[keep[i]].push_back((value - digMin[i]) * scale + physMin[i]);
                }
            }
        }
    }
    cout << "loaded " << path << ": " << rec.names.size() << " channels, " << rec.fs << " Hz, "
         << rec.data[0].size() / rec.fs << " seconds" << endl;
    return true;
}

// each line of brodmann_map.csv is: electrode, ba, area, x, y
// it tells us which brain area is under each electrode and where to draw it
void readBrodmann(string path, Graph &g) {
    g.ba.assign(g.n, -1);
    g.area.assign(g.n, "unknown");
    g.x.assign(g.n, INF);
    g.y.assign(g.n, INF);

    ifstream file(path.c_str());
    if(!file) {
        cout << "cannot open " << path << endl;
        return;
    }
    string line;
    getline(file, line); // skip the header line
    while(getline(file, line)) {
        stringstream ss(line);
        string name, ba, area, x, y;
        getline(ss, name, ',');
        getline(ss, ba, ',');
        getline(ss, area, ',');
        getline(ss, x, ',');
        getline(ss, y, ',');
        for(int i = 0; i < g.n; i++) {
            if(g.names[i] == name) {
                g.ba[i] = stoi(ba);
                g.area[i] = area;
                g.x[i] = stod(x);
                g.y[i] = stod(y);
            }
        }
    }
}
