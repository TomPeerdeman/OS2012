/* This program demonstrates the working of the RSA public key encryption
   algorithm for small prime numbers */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


/* Multiply a and b, modulo m */
unsigned long modmul(unsigned long a, unsigned long b, unsigned long m)
{
    return (a * b) % m;
}

/* Compute x^e modulo m by factoring in x^1, x^2, x^4 etc. when appropriate */
unsigned long modpow(unsigned long x, unsigned long e, unsigned long m)
{
    int i;
    unsigned long r = 1;
    unsigned long p = x;

    for (i = 0; e && (i < 8 * sizeof(unsigned long)); i++)
    {
        if (e & 1) r = modmul(r, p, m);
        p = modmul(p, p, m);
        e >>= 1;
    }
    return r;
}

/* For a given m and e, find a suitable d. return 0 if none found */
unsigned long findD(unsigned long e, unsigned long m)
{
    unsigned long d;
    unsigned long dem1;
    for (d = 1; d < m; d += 2)
    {
        dem1 = d * e - 1;
        if (!(dem1 % m)) return d;
    }
    return 0;
}

int
main()
{
    unsigned long p;
    unsigned long q;
    unsigned long pq;
    unsigned long d;
    unsigned long m;
    unsigned long e = 1;
    unsigned long t;
    unsigned long c;
    unsigned long t2;
    printf("Give p (prime)\n");
    scanf("%lu", &p);
    printf("Give q (prime)\n");
    scanf("%lu", &q);
    pq = p * q;
    m = (p - 1) * (q - 1);
    printf("pq = %lu, (p-1)*(q-1) = %lu\n", pq, m);
    do
    {
        printf("Give e (odd, relatively prime wrt %lu)\n", m);
        printf("Give even value to quit program\n");
        e = 0;
        scanf("%lu", &e);
        if (!(e & 1)) break;
        d = findD(e, m);
        if (d)
        {
            printf("Suggest to use d = %lu\n", d);
            for (t = 0; t < pq; t++)
            {
                c = modpow(t, e, pq);
                t2 = modpow(c, d, pq);
                printf("%lu -> %lu -> %lu\n", t, c, t2);
            }
        } else
        {
            printf("Unsuitable value for e, no d found\n");
        }
    } while (1);
    
    return 0;
}
