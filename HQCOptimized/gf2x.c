#include "gf2x.h"
#include "parameters.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Multiply two polynomials modulo \f$ X^n - 1\f$.
 * This function is optimized for vlight with a low hamming weight
 *
 * This functions multiplies polynomials <b>v1</b> and <b>v2</b>.
 * The multiplication is done modulo \f$ X^n - 1\f$.
 *
 * @param[out] o Product of <b>v1</b> and <b>v2</b>
 * @param[in] vlight Pointer to the first polynomial
 * @param[in] vheavy Pointer to the second polynomial
 */
void PQCLEAN_HQC128_CLEAN_vect_mul_low_weight(uint64_t *o, const uint16_t *vlight, const uint64_t *vheavy, uint16_t weight) {
  uint64_t buffer;
  uint16_t position;
  uint16_t bigposition;
  uint16_t smallposition;
    for (uint8_t i = 0; i < weight; i ++){
      /*
       * Error if an index is further than the size of the vector
       */
      if (vlight[i] > PARAM_N) break;
      /*
       * Get the position of the first bit for the new vector
       */
      position = EMPTY + vlight[i];
      smallposition = position % 64;
      bigposition = (VEC_N_SIZE_64-1) - (position / 64);
      /*
       * Create the first rotated uint64_t in a temporary buffer, then XOR with the result
       */
      if (position + 63 >= (VEC_N_SIZE_64 * 64)){
        buffer = ((vheavy[bigposition] << smallposition) | ((vheavy[VEC_N_SIZE_64-1] << EMPTY) >> (64 - smallposition)) | ((vheavy[VEC_N_SIZE_64-2] >> (64 - smallposition)) >> (64 - EMPTY))) >> EMPTY;
      }
      else{
        buffer = ((vheavy[bigposition] << smallposition) | (vheavy[bigposition-1] >> (64 - smallposition))) >> EMPTY;
      }

      o[(VEC_N_SIZE_64-1)]^=buffer;
      /*
       * Aside from the first unit of the array which behaves differently due to the first few
       * empty bits, the rest goes in a loop.
       *
       * There is a special case when the rotation overlaps with the extremity of the array, the
       * empty bits must be removed and replaced with the correct data
       */
      for (int j = VEC_N_SIZE_64 - 1; j > 0; j--){
        position = (j * 64 + vlight[i]);
        if (position >= (VEC_N_SIZE_64 * 64)){
          position = position - PARAM_N;
        }
        smallposition = position % 64;
        bigposition = (VEC_N_SIZE_64-1) - (position / 64);
        if (position + 63 >= (VEC_N_SIZE_64 * 64)){
          buffer = (vheavy[bigposition] << smallposition);
          buffer |= ((vheavy[VEC_N_SIZE_64-1] << EMPTY) >> (64 - smallposition)) ;
          buffer |= ((vheavy[VEC_N_SIZE_64-2] >> (64 - smallposition)) >> (64 - EMPTY));
        }
        else buffer = (vheavy[bigposition] << smallposition) | (vheavy[bigposition-1] >> (64 - smallposition));
        o[(VEC_N_SIZE_64-1)-j]^=buffer;
      }
      o[VEC_N_SIZE_64 - 1] &= RED_MASK;
    }
}


