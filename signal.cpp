#include "brain.h"

// fast fourier transform (divide and conquer)
// splits the signal into even and odd samples, solves both halves, then joins them
void fft(vector<complex<double>> &a, bool inverse) {
    int n = a.size();
    if(n == 1) return;
    vector<complex<double>> even(n / 2), odd(n / 2);
    for(int i = 0; i < n / 2; i++) {
        even[i] = a[2 * i];
        odd[i] = a[2 * i + 1];
    }
    fft(even, inverse);
    fft(odd, inverse);
    double sign = -1;
    if(inverse) sign = 1;
    for(int k = 0; k < n / 2; k++) {
        complex<double> t = polar(1.0, sign * 2 * PI * k / n) * odd[k];
        a[k] = even[k] + t;
        a[k + n / 2] = even[k] - t;
    }
}

int powerOfTwo(int n) { // since a.size() must be a power of 2
    int p = 1;
    while(p < n) p *= 2;
    return p;
}

// keeps only the frequencies between lo and hi
void bandPass(vector<double> &x, double fs, double lo, double hi) {
    int n = powerOfTwo(x.size());
    vector<complex<double>> a(n, 0.0);
    for(int t = 0; t < x.size(); t++) a[t] = x[t];
    fft(a, false);
    for(int k = 0; k < n; k++) {
        double freq = min(k, n - k) * fs / n; // the second half holds the negative frequencies
        if(freq < lo || freq > hi) a[k] = 0;
    }
    fft(a, true);
    for(int t = 0; t < x.size(); t++) x[t] = a[t].real() / n;
}

// finds the 2 second windows (epochs) without big jumps like blinks or movement
vector<int> cleanEpochs(Recording &rec, int length, double rejectUV) {
    vector<int> starts;
    int samples = rec.data[0].size();
    int skip = rec.fs; // skip the first and last second
    int total = 0;
    for(int start = skip; start + length <= samples - skip; start += length) {
        total++;
        bool clean = true;
        for(int c = 0; c < rec.data.size(); c++) {
            double lowest = rec.data[c][start], highest = rec.data[c][start];
            for(int t = start; t < start + length; t++) {
                lowest = min(lowest, rec.data[c][t]);
                highest = max(highest, rec.data[c][t]);
            }
            if(highest - lowest > rejectUV) clean = false;
        }
        if(clean) starts.push_back(start);
    }
    cout << "epochs kept: " << starts.size() << " of " << total << endl;
    if(starts.empty()) {
        cout << "every epoch was too noisy, try a bigger reject limit" << endl;
        exit(1);
    }
    return starts;
}

// coherency gives one complex number for every pair of electrodes
// its size (0 to 1) is how strongly they are connected
// its angle is their timing: 0 = in step, pi = opposite
Graph coherency(Recording rec, double lo, double hi, double rejectUV) {
    int n = rec.names.size();
    int samples = rec.data[0].size();

    // remove slow drift and fast noise first
    for(int c = 0; c < n; c++) bandPass(rec.data[c], rec.fs, 1, 40);

    // subtract the mean of all channels at every moment to get a common average
    for(int t = 0; t < samples; t++) {
        double mean = 0;
        for(int c = 0; c < n; c++) mean += rec.data[c][t];
        mean /= n;
        for(int c = 0; c < n; c++) rec.data[c][t] -= mean;
    }

    int length = 2 * rec.fs; // 2 second epochs
    int nfft = powerOfTwo(length);
    vector<int> starts = cleanEpochs(rec, length, rejectUV);

    // hann window: fades each epoch in and out so the edges do not add fake frequencies!
    vector<double> hann(length);
    for(int t = 0; t < length; t++) hann[t] = 0.5 - 0.5 * cos(2 * PI * t / (length - 1));

    vector<vector<complex<double>>> cross(n, vector<complex<double>>(n, 0.0));
    vector<double> power(n, 0.0);

    for(int epoch = 0; epoch < starts.size(); epoch++) {
        // spectrum of every channel in this epoch
        vector<vector<complex<double>>> spec(n);
        for(int c = 0; c < n; c++) {
            vector<complex<double>> a(nfft, 0.0);
            for(int t = 0; t < length; t++) a[t] = rec.data[c][starts[epoch] + t] * hann[t];
            fft(a, false);
            spec[c] = a;
        }
        // add up only the frequencies inside our band
        for(int k = 1; k <= nfft / 2; k++) {
            double freq = k * rec.fs / nfft;
            if(freq < lo || freq > hi) continue;
            for(int i = 0; i < n; i++) {
                power[i] += norm(spec[i][k]);
                for(int j = i + 1; j < n; j++) cross[i][j] += spec[i][k] * conj(spec[j][k]);
            }
        }
    }

    Graph g;
    g.n = n;
    g.names = rec.names;
    g.w.assign(n, vector<double>(n, 0.0));
    g.phase.assign(n, vector<double>(n, 0.0));
    for(int i = 0; i < n; i++) {
        for(int j = i + 1; j < n; j++) {
            complex<double> c = cross[i][j] / sqrt(power[i] * power[j]);
            g.w[i][j] = abs(c);
            g.w[j][i] = abs(c);
            g.phase[i][j] = arg(c);
            g.phase[j][i] = -arg(c);
        }
    }
    return g;
}
