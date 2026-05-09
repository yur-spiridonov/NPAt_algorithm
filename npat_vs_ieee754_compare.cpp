#include <iostream>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <intrin.h>
#include <cstdint>
#include <algorithm>
#include <windows.h>

// IEEE 754 decomposition
uint64_t get_mantissa(double x) { uint64_t u; std::memcpy(&u, &x, 8); return u & ((1ULL << 52) - 1); }
uint16_t get_exponent(double x) { uint64_t u; std::memcpy(&u, &x, 8); return (u >> 52) & 0x7FF; }
uint8_t  get_sign    (double x) { uint64_t u; std::memcpy(&u, &x, 8); return (u >> 63) & 1; }

int main() {
    SetThreadAffinityMask(GetCurrentThread(), 1);
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    // [INPUT DATA]
    double        x1_input = 1234;
    double        x2_input = 765432;
    const int64_t Z        = 1000000000LL;

    // --- Print input data ---
    std::cout << "====================================================\n";
    std::cout << " [NPAt vs IEEE 754 — COMPARISON]\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << " [INPUT DATA]\n";
    std::cout << std::scientific << std::setprecision(17);
    std::cout << " Initial Value (X1): " << x1_input << "\n";
    std::cout << " Adder Value   (X2): " << x2_input << "\n";
    std::cout << std::fixed << std::setprecision(0);
    std::cout << " Total Iterations:   Z = " << Z << "\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << " [LAST 5 ITERATIONS]\n";
    std::cout << "====================================================\n";

    // --- IEEE 754 decomposition ---
    uint64_t mant1 = get_mantissa(x1_input);
    uint16_t expo1 = get_exponent(x1_input);
    int      S1    = get_sign(x1_input) ? -1 : 1;

    uint64_t mant2 = get_mantissa(x2_input);
    uint16_t expo2 = get_exponent(x2_input);
    int      S2    = get_sign(x2_input) ? -1 : 1;

    int64_t K1, K2;
    int     E1, E2, E = 0;
    int     g = 0, q = 0, b0 = 0, b1 = 0;
    const int64_t X_max = (1LL << 53);

    if (expo1 == 0) { K1 = (int64_t)mant1;                  E1 = -1022 - 52; }
    else            { K1 = (int64_t)(mant1 | (1ULL << 52)); E1 = (int)expo1 - 1023 - 52; }

    if (expo2 == 0) { K2 = (int64_t)mant2;                  E2 = -1022 - 52; }
    else            { K2 = (int64_t)(mant2 | (1ULL << 52)); E2 = (int)expo2 - 1023 - 52; }

    E = E1;

    // --- Exponent alignment ---
    if (expo1 + expo2 != 0) {
        int t = E1 - E2;
        if (t < 0) {
            E = E2;
            int64_t tempK = K1;
            K1 >>= (std::abs(t) - 1);
            g = (K1 >> 0) & 1;
            if ((K1 << (std::abs(t) - 1)) != tempK) q = 1;
            K1 >>= 1;
        }
        if (t > 0) {
            E = E1;
            int64_t tempK = K2;
            K2 >>= (t - 1);
            g = (K2 >> 0) & 1;
            if ((K2 << (t - 1)) != tempK) q = 1;
            K2 >>= 1;
        }
    }

    // --- Main ALU loop ---
    double final_result_npat = 0.0;
    double X_double_ref      = x1_input;

    for (int64_t i = 2; i <= Z; ++i) {
        K1 = S1 * K1 + S2 * K2;
        S1 = 1;
        if (K1 < 0) { S1 = -1; K1 = std::abs(K1); }

        if (K1 >= X_max) {
            E++;
            b0 = (int)(K1 & 1);
            b1 = (int)((K1 >> 1) & 1);
            K1 >>= 1;
            if (b0 == 1 && !(b1 == 0 && g == 0 && q == 0)) K1++;
            if (g == 1) q = 1;
            g  = (int)(K2 & 1);
            K2 >>= 1;
        } else {
            if (S1 * S2 == 1) {
                if (g == 1 && ((K1 & 1) || q == 1)) K1++;
            } else {
                if (g == 1 && ((K1 & 1) || q == 1)) K1--;
            }
        }

        final_result_npat = S1 * (K1 * std::pow(2.0, E));
        X_double_ref += x2_input;

        // --- Output last 5 iterations ---
        if (i >= Z - 5) {
            std::cout << "i: " << i << "\n";
            std::cout << std::scientific << std::setprecision(50);
            std::cout << " FPU  (IEEE 754): " << X_double_ref      << "\n";
            std::cout << " NPAt (ALU):      " << final_result_npat << "\n";
            std::cout << "----------------------------------------------------\n";
        }
    }

    return 0;
}
