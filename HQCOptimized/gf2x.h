#ifndef GF2X_H
#define GF2X_H
/**
 * @file gf2x.h
 * @brief Header file for gf2x.c
 */

#include <stdint.h>

void PQCLEAN_HQC128_CLEAN_vect_mul(uint64_t *o, const uint64_t *v1, const uint64_t *v2);
void PQCLEAN_HQC128_CLEAN_vect_mul_low_weight(uint64_t *o, const uint16_t *vlight, const uint64_t *vheavy, uint16_t weight);

#endif
