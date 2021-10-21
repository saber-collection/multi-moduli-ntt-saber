#include "api.h"
#include "hal.h"
#include "sendfn.h"

#include "NTT.h"
#include "pack_unpack.h"

#include <stdint.h>
#include <string.h>

extern void __asm_1_to_16(uint16_t*, const uint8_t*);
extern void __asm_3_to_16(uint16_t*, const uint8_t*);
extern void __asm_4_to_16(uint16_t*, const uint8_t*);
extern void __asm_6_to_16(uint16_t*, const uint8_t*);
extern void __asm_10_to_16(uint16_t*, const uint8_t*);
extern void __asm_13_to_16(uint16_t*, const uint8_t*);
extern void __asm_poly_add(uint16_t*, uint16_t*, uint16_t*);


#define MAX(a,b) (((a)>(b))?(a):(b))

#define printcycles(S, U) send_unsignedll((S), (U))

int main(void)
{
  unsigned char key_a[CRYPTO_BYTES], key_b[CRYPTO_BYTES];
  unsigned char sk[CRYPTO_SECRETKEYBYTES];
  unsigned char pk[CRYPTO_PUBLICKEYBYTES];
  unsigned char ct[CRYPTO_CIPHERTEXTBYTES];
  unsigned long long t0, t1;
  int i, j, k;
  int crypto_i;

  hal_setup(CLOCK_BENCHMARK);

  hal_send_str("==========================");

  for(crypto_i=0;crypto_i<ITERATIONS; crypto_i++)
  {
    // Key-pair generation
    t0 = hal_get_time();
    crypto_kem_keypair(pk, sk);
    t1 = hal_get_time();
    printcycles("keypair cycles:", t1-t0);

    // Encapsulation
    t0 = hal_get_time();
    crypto_kem_enc(ct, key_a, pk);
    t1 = hal_get_time();
    printcycles("encaps cycles:", t1-t0);

    // Decapsulation
    t0 = hal_get_time();
    crypto_kem_dec(key_b, ct, sk);
    t1 = hal_get_time();
    printcycles("decaps cycles:", t1-t0);

//

    uint32_t s_NTT[SABER_N];
    uint32_t acc_NTT[SABER_L * SABER_N];
    uint32_t A_NTT[SABER_N];
    uint32_t poly_NTT[SABER_N];
    uint32_t buff_NTT[SABER_N];

    uint16_t poly[SABER_N];
    uint16_t acc[SABER_N];
    uint16_t buff[SABER_N];
    uint16_t s[SABER_N];

    t0 = hal_get_time();
    for (j = 0; j < SABER_L; j++) {

        NTT_forward(s_NTT, poly);

        for (k = 0; k < SABER_L; k++) {

            NTT_forward(A_NTT, poly);

            if (j == 0) {
                NTT_mul(acc_NTT + k * SABER_N, s_NTT, A_NTT);
            } else {
                NTT_mul_acc(acc_NTT + k * SABER_N, s_NTT, A_NTT);
            }

        }
    }

    for (j = 0; j < SABER_L; j++) {
        NTT_inv(poly, acc_NTT + j * SABER_N);
    }
    t1 = hal_get_time();
    printcycles("16-bit MatrixVectorMul speed opt cycles:", t1 - t0);

//

    t0 = hal_get_time();

    for (i = 0; i < SABER_L; i++) {

        for (j = 0; j < SABER_L; j++) {

            NTT_forward1(buff_NTT, buff);

            NTT_forward1(s_NTT, s);

            NTT_mul1(buff_NTT, s_NTT, buff_NTT);
            NTT_inv1(buff_NTT);

            NTT_forward2(poly_NTT, poly);

            NTT_forward2(s_NTT, s);

            NTT_mul2(poly_NTT, s_NTT, poly_NTT);
            NTT_inv2(poly_NTT);

            if(j == 0){
                solv_CRT(acc, buff_NTT, poly_NTT);
            }else{
                solv_CRT(poly, buff_NTT, poly_NTT);
                __asm_poly_add(acc, acc, poly);
            }

        }
    }

    t1 = hal_get_time();
    printcycles("16-bit MatrixVectorMul stack opt cycles:", t1 - t0);

//

    t0 = hal_get_time();
    for(j = 0; j < SABER_L; j++){

        NTT_forward(A_NTT, poly);

        if(j == 0){
            NTT_mul(acc_NTT, s_NTT, A_NTT);
        }else{
            NTT_mul_acc(acc_NTT, s_NTT, A_NTT);
        }

    }

    NTT_inv(acc, acc_NTT);
    t1 = hal_get_time();
    printcycles("16-bit InnderProd(Encrypt) speed opt cycles:", t1-t0);

//

    t0 = hal_get_time();
    for (j = 0; j < SABER_L; j++) {

        NTT_forward(A_NTT, poly);

        NTT_forward(s_NTT, poly);

        if (j == 0) {
            NTT_mul(acc_NTT, A_NTT, s_NTT);
        } else {
            NTT_mul_acc(acc_NTT, A_NTT, s_NTT);
        }
    }

    NTT_inv(poly, acc_NTT);
    t1 = hal_get_time();
    printcycles("16-bit InnderProd(Decrypt) speed opt cycles:", t1-t0);

//

    t0 = hal_get_time();

    for(j = 0; j < SABER_L; j++){

        NTT_forward1(poly_NTT, poly);

        NTT_forward1(s_NTT, s);

        NTT_mul1(buff_NTT, poly_NTT, s_NTT);
        NTT_inv1(buff_NTT);

        NTT_forward2(poly_NTT, poly);

        NTT_forward2(s_NTT, s);

        NTT_mul2(poly_NTT, poly_NTT, s_NTT);
        NTT_inv2(poly_NTT);

        if(j == 0){
            solv_CRT(acc, buff_NTT, poly_NTT);
        }else{
            solv_CRT(buff, buff_NTT, poly_NTT);
            __asm_poly_add(acc, acc, buff);
        }

    }

    t1 = hal_get_time();
    printcycles("16-bit InnderProd stack opt cycles:", t1 - t0);

//

    t0 = hal_get_time();
    NTT_forward(s_NTT, poly);
    t1 = hal_get_time();
    printcycles("two 16-bit NTTs cycles:", t1-t0);

    t0 = hal_get_time();
    NTT_mul(s_NTT, acc_NTT + 0 * SABER_N / 2, acc_NTT + 1 * SABER_N / 2);
    t1 = hal_get_time();
    printcycles("two 16-bit base_mul cycles:", t1-t0);

    t0 = hal_get_time();
    NTT_inv1(s_NTT);
    NTT_inv2(s_NTT);
    t1 = hal_get_time();
    printcycles("two 16-bit iNTT cycles:", t1-t0);

    t0 = hal_get_time();
    solv_CRT(poly, acc_NTT + 0 * SABER_N / 2, acc_NTT + 1 * SABER_N);
    t1 = hal_get_time();
    printcycles("16x16 CRT cycles:", t1-t0);


    if (memcmp(key_a, key_b, CRYPTO_BYTES)) {
      hal_send_str("ERROR KEYS\n");
    }
    else {
      hal_send_str("OK KEYS\n");
    }

    hal_send_str("#");

  }


  return 0;
}