#include "A2B.h"
#include "masksONOFF.h"

#include <stddef.h>

#define pow2(x) (1 << (x))
#define mod2(x) (pow2(x)-1)
#define k 3

struct A2A_C_A_s {
    uint32_t rrr;
    uint32_t GAMMA;
    uint16_t C_A[pow2(k)];
};

// Pre-computation of table
static struct A2A_C_A_s gen_C_A(int n)
{
    int i;
    struct A2A_C_A_s args;

    uint32_t rand = random_uint32();
    uint32_t r = rand & mod2(k);
    uint32_t gamma = rand >> k;

    // gen_C_A
    for (uint32_t A = 0; A < pow2(k); A++) {
        args.C_A[A] = (((A + r) >> k) + gamma);
    }

    uint32_t rrr = 0, GAMMA = 0;

    for (i = 0; i < n; i++) {
        rrr = (rrr | pow2(i * k) * r);
        GAMMA = (GAMMA + pow2((i + 1) * k) * gamma);
    }

    args.rrr = rrr;
    args.GAMMA = GAMMA;

    return args;
}

// Actual conversion
#ifdef SABER_MASKING_ASM
extern void A2A_C_A(const uint16_t *C_A,  uint16_t *Aptr, uint16_t *Rptr, uint32_t rrr, uint32_t GAMMA, uint32_t mod, uint32_t n);
#else
static void A2A_C_A(const uint16_t *C_A,  uint16_t *Aptr, uint16_t *Rptr, uint32_t rrr, uint32_t GAMMA, uint32_t mod, size_t n)
{

    size_t i;
    uint32_t A, R;
    uint32_t A_l, R_l;

    A = *Aptr;
    R = *Rptr;

    A = (A - rrr - GAMMA);

    A = A & mod;
    R = R & mod;

    for (i = 0; i < n; i++) {

        R_l = R & mod2(k);

        A = (A + R_l) & (mod >> (i * k));

        A_l = A & mod2(k);

        A >>= k;
        R >>= k;

        A = (A + C_A[A_l]) & (mod >> ((i + 1) * k));

    }

    *Aptr = A;
    *Rptr = R;
}
#endif

void poly_A2A(uint16_t poly[SABER_SHARES][SABER_N], uint32_t mod, int n)
{
    struct A2A_C_A_s args = gen_C_A(n);

    for (int i = 0; i < SABER_N; i++) {
        A2A_C_A(args.C_A, &poly[0][i], &poly[1][i], args.rrr, args.GAMMA, mod, n);
    }
}