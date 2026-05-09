# NPAt-algorithm

**Software floating-point accumulation on integer ALU — bit-exact IEEE 754, faster than hardware FPU**

U.S. Patent Pending (USPTO № 19/254,239)

This repository is part of the NPAt research project.
Full theoretical foundation: [NPAt-Core-Research](https://github.com/yur-spiridonov/NPAt-Core-Research)

---

## What This Computes

The algorithm computes the sequential accumulation:

```
R = x1 + x2 + x2 + ... + x2    (Z-1 additions of x2)
```

Or equivalently:

```
R = x1 + (Z − 1) · x2
```

Where x1 is the initial value, x2 is the addend, and Z is the total number of iterations.

---

## NPAt Number Format

Any finite floating-point number X is represented as:

```
X̂ = S · K̂ · 2^E
```

Where:
- **S** — sign (±1)
- **K̂** — integer mantissa, `0 < K̂ < 2^t`
- **E** — rounding factor (RF), an integer exponent
- **t** — precision parameter, chosen by the user

The key property: **all parameters are integers**. All arithmetic runs on the integer ALU — no FPU instructions required.

### Precision Parameter t

`t` controls the number of significant bits in the mantissa:

| t | Precision | Equivalent to |
|---|---|---|
| 4 | 4-bit mantissa | minimum |
| 8 | 8-bit mantissa | — |
| 24 | 24-bit mantissa | IEEE 754 float |
| 53 | 53-bit mantissa | IEEE 754 double |
| 4…53 | any | user choice |

The **same algorithm** runs identically for any value of t from 4 to 53 — without changing a single line of the accumulation loop. Only the initialization changes.

> **Note:** The minimum value t=4 is required to represent the smallest decimal digit in binary. The upper limit t=53 corresponds to IEEE 754 double precision. Use of all 64 bits of the register is reserved for a separate algorithm not described here.

---

## 🔧 How to Build and Run

1. Install [Visual Studio](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload
2. Open Visual Studio and select **File → New → Project**
3. Choose **Console App (C++)**, click **Next**, enter a project name, click **Create**
4. Replace all template code in the generated `.cpp` file with the contents of the selected source file (`npat_demo_precision.cpp` or `npat_benchmark.cpp`)
5. In the toolbar, set configuration to **Release** and platform to **x64**
6. Press **Ctrl+F5** to build and run without debugger

> ⚠️ Requires Windows and MSVC compiler. The benchmark uses `<intrin.h>` and `<windows.h>` — GCC/Clang are not supported.

---

## Part 1 — Precision Demo

### Source: [npat_demo_precision.cpp](npat_demo_precision.cpp)

This program demonstrates a key property of NPAt: **the same algorithm runs at any precision from t=4 to t=53 without modification**.

In IEEE 754, switching between float and double requires different data types, different function signatures, and different code paths. For AI chips, the transition from high-precision training (FP32/FP64) to low-precision inference (FP8/FP16) is a significant engineering challenge requiring separate hardware blocks or precision conversion layers.

In NPAt, precision is a single integer parameter `t`. The accumulation loop is identical for all values of t — only the initialization changes. This means a single hardware implementation could natively support the full range from training to inference precision without switching modes.

**How to set input data:**

```cpp
double  x1 = -3.45678998765432109;   // initial value
double  x2 =  9.87654321234567852;   // addend
int64_t Z  = 1000;                   // total iterations
```

The program automatically runs for t = 8, 12, 16, 24, 32, 40, 48, 53.

**How to verify the result with a calculator:**

The exact result can be computed as:
```
R = x1 + (Z − 1) · x2
R = −3.45678998... + 999 × 9.87654321... = 9863.209879...
```

Use any scientific calculator (Windows Calc in Scientific mode, or WolframAlpha) to verify.

**Sample output:**

```
====================================================
 NPAt-algorithm: variable precision demo
====================================================
 X1 = -3.456790e+00
 X2 =  9.876543e+00
 Z  = 1000
----------------------------------------------------
 Exact result:   9.86320987914567741e+03
====================================================
  t  |        NPAt result
-----|-----------------------------------------
   8 | 4.0960000000e+03
  12 | 9.6240000000e+03
  16 | 9.8830000000e+03
  24 | 9.8632343750e+03
  32 | 9.8632100563e+03
  40 | 9.8632098773e+03
  48 | 9.8632098791e+03
  53 | 9.8632098791e+03
====================================================
```

**Observations:**
- At t=53 the result matches the exact value to all available decimal places
- At t=48 the result is already identical to t=53 — this is the precision limit of double for this input
- Each additional ~5 bits of t yields approximately 1–2 more correct decimal digits
- At t=8 the algorithm still runs correctly — it simply operates at 8-bit precision

### Building via command line

```bash
cl /O2 /EHsc npat_demo_precision.cpp /Fe:npat_demo_precision.exe
npat_demo_precision.exe
```

---

## Part 2 — Benchmark

### Source: [npat_benchmark.cpp](npat_benchmark.cpp)

This program measures the performance of NPAt-algorithm against the hardware IEEE 754 baseline.

**How to set input data:**

```cpp
double  x1_input = -3.45678998765432109e200;  // initial value
double  x2_input =  9.87654321234567852e200;  // addend
int64_t Z        = 1000000000LL;              // total iterations (10^9)
int     t        = 53;                        // precision parameter
```

**What the output shows:**

```
====================================================
 [NPAt-ALGORITHM — ALU ENGINE]
----------------------------------------------------
 [INPUT DATA]
 Initial Value (X1): -3.45678998765432109e+200
 Adder Value   (X2):  9.87654321234567852e+200
 Total Iterations:   Z = 1000000000
 Precision (t):      53 bits
 Mode:               VOLATILE (Full Normalization)
----------------------------------------------------
 [RESULT]
 NPAt FINAL SUM:
 9.87654328996929952145905990045930686407423335708138e+209
----------------------------------------------------
 [PERFORMANCE]
 TOTAL CPU CYCLES:   2593917812
 CYCLES PER ITER:    2.5939
 Med cycles/iter:    2.6146
 Spread:             14.1802 %
====================================================
```

**How to compare with hardware:**

Run the hardware baseline from [Benchmark_Hardware-vs-NPAt-](https://github.com/yur-spiridonov/Benchmark_Hardware-vs-NPAt-) with the same X1 and X2 values. Compare:
- **FINAL SUM** — must be bit-exact identical (verified to 50 decimal places)
- **CYCLES PER ITER** — NPAt should be faster

**How to verify the result with WolframAlpha:**

```
-3.45678998765432109e200 + 999999999 * 9.87654321234567852e200
```

### Building via command line

```bash
cl /O2 /fp:precise /EHsc npat_benchmark.cpp /Fe:npat_benchmark.exe
npat_benchmark.exe
```

> **Note:** Results are hardware- and compiler-specific. Reference results above were obtained on a specific x86-64 machine with MSVC /O2, Windows 11, Core 0, REALTIME_PRIORITY_CLASS.

---

## Benchmark Results

Full verified results across 5 input types:

| Input Type | HW cycles/iter | NPAt cycles/iter | Speedup |
|---|:---:|:---:|:---:|
| Subnormal (−9.999e−311) | 5.2147 | 2.4553 | **×2.12** |
| Normal fractional (0.125 / 0.625) | 5.2580 | 2.3242 | **×2.26** |
| Large integers (e+06 / e+09) | 5.4411 | 2.5621 | **×2.12** |
| Extreme magnitude (e+200) | 5.2394 | 2.6708 | **×1.96** |
| Small normal, subtraction | 5.4773 | 3.7429 | **×1.46** |

All tests: Z = 1,000,000,000 · MSVC /O2 · Windows 11 · Core 0 · REALTIME_PRIORITY_CLASS

Full results with screenshots: [Benchmark_Hardware-vs-NPAt-](https://github.com/yur-spiridonov/Benchmark_Hardware-vs-NPAt-)

---

## Related Repositories

| Repository | Description |
|---|---|
| [NPAt-Core-Research](https://github.com/yur-spiridonov/NPAt-Core-Research) | Full theoretical foundation of NPAt format |
| [Benchmark_Hardware-vs-NPAt-](https://github.com/yur-spiridonov/Benchmark_Hardware-vs-NPAt-) | Hardware IEEE 754 baseline — full results with screenshots |
| [PresentationNPat](https://github.com/yur-spiridonov/PresentationNPat) | Precision comparison results: NPAt vs IEEE 754 across multiple input types |

---

*Released for research and evaluation. The NPAt format and associated algorithms are covered by U.S. Patent Pending (USPTO № 19/254,239). Commercial use requires a licensing agreement.*
