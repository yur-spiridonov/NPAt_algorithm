#include <iostream>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <algorithm>

uint64_t get_mantissa(double x) { uint64_t u; std::memcpy(&u, &x, 8); return u & ((1ULL << 52) - 1); }
uint16_t get_exponent(double x) { uint64_t u; std::memcpy(&u, &x, 8); return (u >> 52) & 0x7FF; }
uint8_t  get_sign    (double x) { uint64_t u; std::memcpy(&u, &x, 8); return (u >> 63) & 1; }

double npat_sum(double x1_input, double x2_input, int64_t Z, int t) {

    const int64_t K_max = (1LL << t);

    uint64_t mant1 = get_mantissa(x1_input);
    uint16_t expo1 = get_exponent(x1_input);
    int      S1    = get_sign(x1_input) ? -1 : 1;

    uint64_t mant2 = get_mantissa(x2_input);
    uint16_t expo2 = get_exponent(x2_input);
    int      S2    = get_sign(x2_input) ? -1 : 1;

    int64_t K1, K2;
    int     E1, E2, E;
    int     g = 0, q = 0, b0 = 0, b1 = 0;

    // --- Initialization: IEEE 754 → NPAt format ---
    if (expo1 == 0) {
        K1 = (int64_t)(mant1 >> (52 - t));
        E1 = -1022 - t;
    } else {
        K1 = (int64_t)((mant1 | (1ULL << 52)) >> (53 - t));
        E1 = (int)expo1 - 1022 - t;
    }

    if (expo2 == 0) {
        K2 = (int64_t)(mant2 >> (52 - t));
        E2 = -1022 - t;
    } else {
        K2 = (int64_t)((mant2 | (1ULL << 52)) >> (53 - t));
        E2 = (int)expo2 - 1022 - t;
    }

    E = E1;

    // --- Exponent alignment ---
    if (E1 != E2) {
        int shift = E1 - E2;
        if (shift < 0) {
            E = E2;
            int64_t tempK = K1;
            int s = std::abs(shift);
            K1 >>= (s - 1);
            g = (int)(K1 & 1);
            if ((K1 << (s - 1)) != tempK) q = 1;
            K1 >>= 1;
        } else {
            E = E1;
            int64_t tempK = K2;
            K2 >>= (shift - 1);
            g = (int)(K2 & 1);
            if ((K2 << (shift - 1)) != tempK) q = 1;
            K2 >>= 1;
        }
    }

    // --- Accumulation loop ---
    for (int64_t i = 2; i <= Z; ++i) {
        K1 = S1 * K1 + S2 * K2;  S1 = 1;
        if (K1 < 0) { S1 = -1;  K1 = std::abs(K1); }

        if (K1 >= K_max) {
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
    }

    return S1 * (K1 * std::pow(2.0, E));
}

int main() {

    // [INPUT DATA]
    double  x1 = -3.45678998765432109;
    double  x2 =  9.87654321234567852;
    int64_t Z  = 1000;

    // Exact result (double precision reference)
    double exact = x1 + (Z - 1) * x2;

    std::cout << "====================================================\n";
    std::cout << " NPAt-algorithm: variable precision demo\n";
    std::cout << "====================================================\n";
    std::cout << std::scientific << std::setprecision(6);
    std::cout << " X1 = " << x1 << "\n";
    std::cout << " X2 = " << x2 << "\n";
    std::cout << " Z  = " << Z  << "\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << " Exact result:   " << std::setprecision(17) << exact << "\n";
    std::cout << "====================================================\n";
    std::cout << "  t  |        NPAt result\n";
    std::cout << "-----|-----------------------------------------\n";

    for (int t : {4, 8, 12, 16, 24, 32, 40, 48, 53}) {
        double result = npat_sum(x1, x2, Z, t);
        std::cout << std::fixed << std::setprecision(0);
        std::cout << "  " << std::setw(2) << t << " | ";
        std::cout << std::scientific << std::setprecision(10) << result << "\n";
    }

    std::cout << "====================================================\n";
    return 0;
}
