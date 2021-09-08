#ifndef GEN_TABLE_H
#define GEN_TABLE_H

#include "tools.h"

void gen_CT_table(int *des, int scale, int _omega, int _Q);
void gen_inv_CT_table(int *des, int scale, int _omega, int _Q);

void gen_twist_table(int *des, int scale, int _omega, int _Q);
void gen_CT_negacyclic_table(int *des, int scale, int _omega, int _Q);

void gen_mul_table(int *des, int scale, int _omega, int _Q);
void gen_all_mul_table(int *des, int scale, int _omega, int _Q);

void gen_streamlined_CT_table(int *des, int scale, int _omega, int _Q, struct compress_profile *_profile, int pad);
void gen_streamlined_CT_negacyclic_table(int *des, int scale, int _omega, int _Q, struct compress_profile *_profile, int pad);
void gen_streamlined_inv_CT_table(int *des, int scale, int _omega, int _Q, struct compress_profile *_profile, int pad);

void gen_streamlined_inv_CT_negacyclic_table(int *des, int scale1, int _omega, int scale2, int twist_omega, int _Q, struct compress_profile *_profile, int pad);

#endif