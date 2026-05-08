#include <iostream>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <intrin.h>
#include <cstdint>
#include <algorithm>
#include <windows.h>

uint64_t get_mantissa(double x) { uint64_t u; std::memcpy(&u, &x, 8); return u & ((1ULL << 52) - 1); }
uint16_t get_exponent(double x) { uint64_t u; std::memcpy(&u, &x, 8); return (u >> 52) & 0x7FF; }
uint8_t get_sign(double x) { uint64_t u; std::memcpy(&u, &x, 8); return (u >> 63) & 1; }

int main() {
    SetThreadAffinityMask(GetCurrentThread(), 1);
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    // [INPUT DATA]
    double x1_input = -3.45678998765432109e200;
    double x2_input = 9.87654321234567852e200;
    const int64_t Z = 1000000000LL;
    const int RUNS = 9;
    uint64_t results[RUNS];
    double final_result_npat = 0;

    for (int run = 0; run < RUNS; run++) {
        uint64_t mant1 = get_mantissa(x1_input);
        uint16_t expo1 = get_exponent(x1_input);
        int S1 = get_sign(x1_input) ? -1 : 1;
        uint64_t mant2 = get_mantissa(x2_input);
        uint16_t expo2 = get_exponent(x2_input);
        int S2 = get_sign(x2_input) ? -1 : 1;

        int64_t K1, K2;
        int E1, E2, E = 0;
   
        int g = 0, q = 0, b0 = 0, b1 = 0;
        int64_t X_max = (1LL << 53);
        int64_t i;
        if (expo1 == 0) { K1 = (int64_t)mant1; E1 = -1022 - 52; }
        else { K1 = (int64_t)(mant1 | (1ULL << 52)); E1 = (int)expo1 - 1023 - 52; }
        if (expo2 == 0) { K2 = (int64_t)mant2; E2 = -1022 - 52; }
        else { K2 = (int64_t)(mant2 | (1ULL << 52)); E2 = (int)expo2 - 1023 - 52; }

        E = E1;
        if (expo1 + expo2 != 0) {
            int t = E1 - E2;
            if (t < 0) {
                E = E2; int64_t tempK = K1;
                K1 >>= (std::abs(t) - 1);
                g = (K1 >> 0) & 1;
                if ((K1 << (std::abs(t) - 1)) != tempK) q = 1;
                K1 >>= 1;
            }
            if (t > 0) {
                E = E1; int64_t tempK = K2;
                K2 >>= (t - 1);
                g = (K2 >> 0) & 1;
                if ((K2 << (t - 1)) != tempK) q = 1;
                K2 >>= 1;
            }
        }

        _mm_lfence();
        uint64_t start = __rdtsc();
        _mm_lfence();

        for ( i = 2; i <= Z; ++i) {
            K1 = S1 * K1 + S2 * K2; S1 = 1;
            if (K1 < 0) { S1 = -1; K1 = std::abs(K1); }
            if (K1 >= X_max) {
                E++;
                b0 = (K1 & 1); b1 = ((K1 >> 1) & 1);
                K1 >>= 1;
                if (b0 == 1 && !(b1 == 0 && g == 0 && q == 0)) K1++;
                if (g == 1) q = 1;
                g = (K2 & 1);
                K2 >>= 1;
            }
            else {
                if (S1 * S2 == 1) {
                    if (g == 1 && ((K1 & 1) || q == 1))
                        K1++;
              
                }
                else {
                    if (g == 1 && ((K1 & 1) || q == 1)) K1--;
                }
            }
        }

        _mm_lfence();
        uint64_t end = __rdtsc();
        _mm_lfence();

        results[run] = end - start;
        final_result_npat = S1 * (K1 * std::pow(2.0, E));
    }

    std::sort(results, results + RUNS);

    std::cout << "====================================================\n";
    std::cout << " [NPAt PATHWAY 1 - ALU ENGINE (RESEARCH MODE)]\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << "[INPUT DATA]\n";
    std::cout << std::scientific << std::setprecision(17);
    std::cout << "Initial Value (X1): " << x1_input << "\n";
    std::cout << "Adder Value (X2):   " << x2_input << "\n";
    std::cout << std::fixed << std::setprecision(0);
    std::cout << "Total Iterations:   Z = " << Z << "\n";
    std::cout << "Mode:               VOLATILE (Full Normalization)\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << "NPAt (FINAL SUM):\n";
    std::cout << std::scientific << std::setprecision(50) << final_result_npat << "\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << "[PERFORMANCE]\n";
    std::cout << std::fixed << std::setprecision(0);
    std::cout << "TOTAL CPU CYCLES: " << results[0] << "\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "CYCLES PER ITER:  " << (double)results[0] / Z << "\n";
    std::cout << "Med cycles/iter:  " << (double)results[RUNS / 2] / Z << "\n";
    std::cout << "Spread:           " << 100.0 * (results[RUNS - 1] - results[0]) / results[0] << " %\n";
    std::cout << "====================================================\n";

    return 0;
}
