#include <iostream>
#include <iomanip>
#include <intrin.h>
#include <cstdint>
#include <algorithm>
#include <windows.h>

int main() {
    // OS Environment Stabilization
    SetThreadAffinityMask(GetCurrentThread(), 1);
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    // [INPUT DATA]
    double x1 = 1.00000000000000003e-300;
    double x2 = -9.99999999999996945e-312;
    const int64_t Z = 1000000000LL;
    const int RUNS = 9;
    uint64_t results[RUNS];
    double final_acc = 0;

    // Cache Warm-up Phase
    {
        volatile double acc = x1;
        for (int64_t i = 2; i <= Z; ++i) acc += x2;
    }

    // Benchmark Execution
    for (int run = 0; run < RUNS; run++) {
        volatile double acc = x1;

        _mm_lfence();
        uint64_t start = __rdtsc();
        _mm_lfence();

        for (int64_t i = 2; i <= Z; ++i) acc += x2;

        _mm_lfence();
        uint64_t end = __rdtsc();
        _mm_lfence();

        results[run] = end - start;
        final_acc = acc;
    }

    std::sort(results, results + RUNS);

    double min_c = (double)results[0] / Z;
    double med_c = (double)results[RUNS / 2] / Z;
    double spread = 100.0 * (results[RUNS - 1] - results[0]) / results[0];

    // Symmetric Reporting Format
    std::cout << "====================================================\n";
    std::cout << " [HARDWARE IEEE 754 BASELINE]\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << " [INPUT DATA]\n";
    std::cout << std::scientific << std::setprecision(17);
    std::cout << " Initial Value (X1): " << x1 << "\n";
    std::cout << " Adder Value (X2):   " << x2 << "\n";
    std::cout << std::fixed << std::setprecision(0);
    std::cout << " Total Iterations:   Z = " << Z << "\n";
    std::cout << " Mode:               VOLATILE (L1 Latency)\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << " [RESULTS]\n";
    std::cout << " FPU (FINAL SUM):\n";
    std::cout << std::scientific << std::setprecision(50) << final_acc << "\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << " [PERFORMANCE]\n";
    std::cout << std::fixed << std::setprecision(0);
    std::cout << " TOTAL CPU CYCLES:   " << results[0] << "\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << " CYCLES PER ITER:    " << min_c << "\n";
    std::cout << " Med cycles/iter:    " << med_c << "\n";
    std::cout << " Spread:             " << spread << " %\n";
    std::cout << "====================================================\n";

    return 0;
}
