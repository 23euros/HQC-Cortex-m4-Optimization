#ifndef VECTOR_H
#define VECTOR_H

/**
 * @file vector.h
 * @brief Header file for vector.c
 */
#include "shake_prng.h"
#include <stddef.h>
#include <stdint.h>
#include <parameters.h>

#if !defined(MUL) || !defined (ADD)
void PQCLEAN_HQC128_CLEAN_vect_set_random_fixed_weight(seedexpander_state *ctx, uint64_t *v, uint16_t weight);
#endif

#if defined(MUL) || defined(ADD)
void PQCLEAN_HQC128_CLEAN_vect_set_random_fixed_weight_indexes(seedexpander_state *ctx, uint16_t weight, uint16_t *indexes);
#endif

void PQCLEAN_HQC128_CLEAN_vect_set_random(seedexpander_state *ctx, uint64_t *v);

void PQCLEAN_HQC128_CLEAN_vect_add(uint64_t *o, const uint64_t *v1, const uint64_t *v2, size_t size);

void PQCLEAN_HQC128_CLEAN_vect_add_light(uint64_t *o, const uint16_t *indexes, uint16_t weight, uint16_t size);

uint8_t PQCLEAN_HQC128_CLEAN_vect_compare(const uint8_t *v1, const uint8_t *v2, size_t size);

void PQCLEAN_HQC128_CLEAN_vect_resize(uint64_t *o, uint32_t size_o, const uint64_t *v, uint32_t size_v);

#endif
