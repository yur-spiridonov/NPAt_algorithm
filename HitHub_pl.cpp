// =============================================================================
// Author: Iouri Spiridonov
// NPAt — Number with Point After t

// =============================================================================

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>

using namespace std;

int main() {
    // ========================== INPUT ==========================
    double x1 = -89382715639;
    double x2 = 7878271567;
    int Z = 1000000;

    std::cout << "x1 : " << x1 << "\n";
    std::cout << "x2 : " << x2 << "\n";
    std::cout << "Z : " << Z << "\n";
    std::cout << "" << "\n";


    // ========================== CONFIG ==========================
    int Double_Float = 0;                    // 0 = double (p=53), 1 = float (p=24)
    int p = (Double_Float == 0) ? 53 : 24;

    long long int K, K1, K2, X_max;
    int e1, e2, E, v1, v2, S1 = 1, S2 = 1;
    int g = 0, q = 0, bit0 = 0, bit1, t;
    double buffer;

    double Y1_Double = 0, Y2_Double = 0, R_Double = 0;
    float  Y1_Float = 0, Y2_Float = 0, R_Float = 0;

    // ========================== INITIALIZATION ==========================
    if (x1 < 0) S1 = -1;
    if (x2 < 0) S2 = -1;

    frexp(fabs(x1), &e1);
    frexp(fabs(x2), &e2);

    t = e1 - e2;
    if (t < 0) {
        t = -t;
        buffer = x1; x1 = x2; x2 = buffer;
        buffer = e1; e1 = e2; e2 = buffer;
        buffer = S1; S1 = S2; S2 = buffer;
    }

    if (Double_Float == 0) {
        Y1_Double = x1;
        Y2_Double = x2;

    }
    else {
        Y1_Float = (float)x1;
        Y2_Float = (float)x2;
    }

    X_max = (1LL << p);
    v1 = e1 - p;
    v2 = e2 - p;

    // ========================== CONVERT TO INTEGER MANTISSA ==========================
    if (Double_Float == 1) {
        K1 = llround(fabs(x1) * pow(2.0, -v1 + 1));
        K2 = llround(fabs(x2) * pow(2.0, -v2 + 1));
        bit0 = K1 & 1; K1 = K1 >> 1; if (bit0) K1++;
        bit0 = K2 & 1; K2 = K2 >> 1; if (bit0) K2++;
    }
    else {
        K1 = llround(fabs(x1) * pow(2.0, -v1));
        K2 = llround(fabs(x2) * pow(2.0, -v2));
    }

    // ========================== EXPONENT ALIGNMENT ==========================
    if (t != 0) {
        K2 = K2 >> (t - 1);
        bit0 = (K2 >> 0) & 1;
        K2 = K2 >> 1;
        if (~(K2 << t) * x2 != 0) q = 1;
    }
    if (bit0 == 1) K2++;//    g = 1;

    E = v1;
    K = K1;

    // ========================== SUMMATION LOOP ==========================
//    cout << fixed << setprecision(10);
    for (int i = 2; i <= Z; ++i) {
        K = S1 * K + S2 * K2;
        S1 = 1;
        if (K < 0) { S1 = -1; K = abs(K); }

        if (abs(K) >= X_max) {
            E++;
            if (g == 1) q = 1;
            bit0 = (K >> 0) & 1;
            bit1 = (K >> 1) & 1;
            if (bit1 == 1) {
                if (bit0 == 1) K++;
            }
            else {
                if ((bit0 | q) == 1) K++;
            }
            g = 0;
            bit0 = (K2 >> 0) & 1;
            if (bit0 == 1) g = 1;
            K = K >> 1;
            K2 = K2 >> 1;
        }
        else {
            bit0 = (K >> 0) & 1;
            if (bit0 == 1)
                K = K + g;
            else if ((g + q) / 2 == 1)
                K = K + 1;
        }

        if (Double_Float == 0) {
            R_Double = S1 * (K * pow(2.0, E));
            Y1_Double = Y1_Double + Y2_Double;
        }
        else {
            R_Float = S1 * (K * pow(2.0, E));
            Y1_Float = Y1_Float + Y2_Float;
        }
 
        if (i <= 10 || i % 100000 == 0 || i == Z) {
            cout << "i=" << setw(3) << i
                << " | NPAp = " << setw(22) << (Double_Float ? R_Float : R_Double)
                << " | IEEE = " << setw(22) << (Double_Float ? (double)Y1_Float : Y1_Double)
                << " | Match: " << ((Double_Float ? fabs(R_Float - Y1_Float) : fabs(R_Double - Y1_Double)) < 1e-6 ? "YES" : "NO")
                << "\n";
        }
    }

    return 0;

}
