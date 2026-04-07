// Author: Iouri Spiridonov
// NPAp — Number with Point After p
//Full, accurate, bit-for-bit compatibility with IEEE 754 double/float
// =============================================================================

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <bitset>

using namespace std;

int main()
{
    // ========================== INPUT ==========================
    double x1 = 0.93827156398976e-38;
    double x2 = 0.78782715676579e-38;
    int Z = 1000000000;

     std::cout << "  x1  = " << x1 << "\n";
     std::cout << "  x2 = " << x2 << "\n";
     std::cout << "  Z = " << Z << "\n";
     std::cout << "  NPAt =  x1 + (x2 + x2 + ... + x2)"  << "\n";

    // ========================== CONFIG ==========================
    int Double_Float = 0;  // 0 = double (p=53), 1 = float (p=24)
    int p = (Double_Float == 0) ? 53 : 24;

    long long int K, K1, K2, X_max;
    int e1, e2, E, v1, v2;
    int S1 = 1, S2 = 1;
    int g = 0, q = 0, bit0 = 0, bit1 = 0, t = 0;
    unsigned i;
    double buffer = 0.0;
    double Y1_Double = 0.0, Y2_Double = 0.0, R_Double = 0.0;
    float  Y1_Float = 0.0f, Y2_Float = 0.0f, R_Float = 0.0f;

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
        Y1_Float = static_cast<float>(x1);
        Y2_Float = static_cast<float>(x2);
    }

    X_max = (1LL << p);
    v1 = e1 - p;
    v2 = e2 - p;

    // ========================== CONVERT TO INTEGER MANTISSA ==========================
    if (Double_Float == 1) 
    {
        K1 = llround(fabs(x1) * pow(2.0, -v1 + 1));
        K2 = llround(fabs(x2) * pow(2.0, -v2 + 1));

        bit0 = K1 & 1;
        K1 >>= 1;
        if (bit0) K1++;

        bit0 = K2 & 1;
        K2 >>= 1;
        if (bit0) K2++;

    }
    else {
        K1 = llround(fabs(x1) * pow(2.0, -v1));
        K2 = llround(fabs(x2) * pow(2.0, -v2));
    }

    // ========================== EXPONENT ALIGNMENT ==========================
    if (t != 0) 
    {
        K = K2;
        K2 >>= (t - 1);
        g = (K2 >> 0) & 1;
        if (K2 << (t - 1) != K)
            q = 1;   
    bit0 = (K2 >> 0) & 1;
        K2 >>= 1;
    }

    if (g == 1&&bit0==1) {
        K2++;
    }

    E = v1;
    K = K1;

    // ========================== SUMMATION LOOP ==========================

    for ( i = 2; i <= Z; ++i)
    {
      
        K = S1 * K + S2 * K2;
        S1 = 1;

        if (K < 0) 
        {
            S1 = -1;
            K = abs(K);
        }

         bit0 = (K >> 0) & 1;

        if (abs(K) >= X_max)
        {
            E++;

            bit1 = (K >> 1) & 1;
            K = K >> 1;

            if (bit0 == 1)
            {
                if (bit0 == 1 && bit1 == 0 && g == 0 && q == 0)
                    goto  ResearchK2;

                if (bit0 == 1 && bit1 == 1)
                {
                    K++;
                    goto  ResearchK2;
                }

                K++;
            }
        ResearchK2:
            bit0 = (K2 >> 0) & 1;
            if (g == 1)
                q = 1;
            g = 0;
            if (bit0 == 1)
                g = 1;
            K2 = K2 >> 1;
        }
        else
        { 
            if (g == 1)
            {
                if (bit0 == 1)
                    K++;
                  else
                { 
                    if (q == 1)
                        K++;
                }
            }
}

            if (Double_Float == 0)
            {
                R_Double = S1 * (K * pow(2.0, E));
            }
            else
            {
                R_Float = S1 * (K * pow(2.0, E));
                Y1_Float += Y2_Float;
            }
    
             Y1_Double += Y2_Double;

  
             if (i >=  Z-10 )
        
            {
           std::cout << "  ------------------------------ " << "\n";
                std::cout  << "  i  = " << i << "\n";
               std::cout << setprecision(55) << "  NPAt     =  " << R_Double << "\n";
              std::cout << setprecision(55) << "  Y_double  = " << Y1_Double << "\n";
     
            }


    }

    return 0;
}
