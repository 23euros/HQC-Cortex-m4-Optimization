/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "api.h"
#include "rt_test_root.h"
#include "oslib_test_root.h"
#include <chprintf.h>
#include <stdlib.h>
#include <string.h>
#include "gf2x.h"
#include "parameters.h"

/* Define the Thread parameters */
#if defined(MUL) && defined(ADD)
#define WORKBUF_SIZE 50000
#elif !defined(MUL) && !defined(ADD)
#define WORKBUF_SIZE 85000
#elif defined(MUL)
#define WORKBUF_SIZE 60000
#else
#define WORKBUF_SIZE 80000
#endif

/* Create a mutex and the interruptions counter */
static MUTEX_DECL(mutex);

/* Create the thread working areas */
static THD_WORKING_AREA(waThread1, WORKBUF_SIZE);

/* Define the thread function */
static THD_FUNCTION(Thread1, arg) {
    (void)arg;
    chRegSetThreadName("dilithium2_sign");

    /* Initialize the parameters for the signature */
    uint8_t ciphertext[CRYPTO_CIPHERTEXTBYTES];
    uint8_t ciphertext_received[CRYPTO_CIPHERTEXTBYTES];
    uint8_t shared_secret[CRYPTO_BYTES];
    uint8_t shared_secret_received[CRYPTO_BYTES];
    uint8_t public_key[CRYPTO_PUBLICKEYBYTES];
    uint8_t secret_key[CRYPTO_SECRETKEYBYTES];

    /* Initialize the parameters for cycle count and timer */
    uint32_t cycle_counter_keygen;
    uint32_t cycle_counter_enc;
    uint32_t cycle_counter_dec;
    uint32_t start_cycle;
    uint32_t end_cycle;
    systime_t start_time;
    systime_t end_time;
    systime_t timer_keygen;
    systime_t timer_enc;
    systime_t timer_dec;
    uint16_t loops = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

      loops = 0;
//      chprintf((BaseSequentialStream *)&SD2,"pk: %lu - sk: %lu\r\n", public_key, secret_key);
      while (loops<10000) {
          DWT->CYCCNT = 0;
          loops ++;

          /* Set the counters */
          palSetPad(GPIOD, GPIOD_LED3);       // Orange.
          chSysLock();
          start_time = chVTGetSystemTimeX();
          start_cycle = DWT->CYCCNT;
          chSysUnlock();

          /* Generate key pair */
          crypto_kem_keypair(public_key, secret_key);

          /* Update the counter and metrics */
          chSysLock();
          end_cycle = DWT->CYCCNT;
          end_time = chVTGetSystemTimeX();
          chSysUnlock();
          palClearPad(GPIOD, GPIOD_LED3);     // Orange.
          cycle_counter_keygen = end_cycle - start_cycle;
          timer_keygen = end_time - start_time;

          /* Generate a random message */
          for (int i = 0; i < CRYPTO_BYTES; i++) {
              shared_secret[i] = rand() % 256;
          }

          /* Set the counters */
          palSetPad(GPIOD, GPIOD_LED3);       // Orange.
          chSysLock();
          start_time = chVTGetSystemTimeX();
          start_cycle = DWT->CYCCNT;
          chSysUnlock();

          /* Encapsulate the secret */
          crypto_kem_enc(ciphertext, shared_secret, public_key);

          /* Update the counter and metrics */
          chSysLock();
          end_cycle = DWT->CYCCNT;
          end_time = chVTGetSystemTimeX();
          chSysUnlock();
          palClearPad(GPIOD, GPIOD_LED3);     // Orange.
          cycle_counter_enc = end_cycle - start_cycle;
          timer_enc = end_time - start_time;

          for (int i = 0; i < CRYPTO_CIPHERTEXTBYTES; i++){
            ciphertext_received[i] = ciphertext[i];
          }
          for (int i = 0; i < CRYPTO_BYTES; i++){
            shared_secret_received[i] = 0;
          }

          /* Set the counters and launch concurrent thread */
          palSetPad(GPIOD, GPIOD_LED3);       // Orange.
          chSysLock();
          start_time = chVTGetSystemTimeX();
          start_cycle = DWT->CYCCNT;
          chSysUnlock();

          /* Decapsulate the secret */
          crypto_kem_dec(shared_secret_received, ciphertext_received, secret_key);

          /* Update the counter and metrics, stop the concurrent thread */
          chSysLock();
          end_cycle = DWT->CYCCNT;
          end_time = chVTGetSystemTimeX();
          chSysUnlock();
          palClearPad(GPIOD, GPIOD_LED3);     // Orange.
          cycle_counter_dec = end_cycle - start_cycle;
          timer_dec = end_time - start_time;

          /* Display the current loop results */
          chMtxLock(&mutex);
          chprintf((BaseSequentialStream *)&SD2,"%lu,%lu,%lu,%lu,%lu,%lu,%lu\r\n", loops, cycle_counter_keygen, timer_keygen, cycle_counter_enc, timer_enc, cycle_counter_dec, timer_dec);
          chMtxUnlock(&mutex);
//          chThdExit(MSG_RESET);

          if (memcmp(shared_secret, shared_secret_received, sizeof(shared_secret)) != 0) {
            chMtxLock(&mutex);
            chprintf((BaseSequentialStream *)&SD2, "Error - Wrong KEM at loop %lu \r\n", loops);
            chMtxUnlock(&mutex);
            chThdExit(MSG_RESET);
          }



      }
    /* Display Final Results */
    chMtxLock(&mutex);
//    chprintf((BaseSequentialStream *)&SD2, "Test finished - Final Results \r\n\r\n - Cycles - \r\nAverage: %lu \r\nMinimum: %lu \r\nMaximum %lu \r\n\r\n - Time - \r\nAverage: %lu ms \r\nMinimum: %lu ms \r\nMaximum: %lu ms", (uint32_t) avg, min, max, (systime_t) avg_t, min_t, max_t);
    chMtxUnlock(&mutex);
    chThdExit(MSG_OK);
}


/*
 * Application entry point.
 */

int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */

  /* Initialize ChibiOS */
  halInit();
  chSysInit();

  sdStart(&SD1, NULL);
  sdStart(&SD2, NULL);
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));


  /*
   * Creates the example thread.
   */

  chThdCreateStatic(waThread1, sizeof(waThread1), LOWPRIO, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */

  /* Infinite loop */
  while (true) {
    if (palReadPad(GPIOA, GPIOA_BUTTON)) {
      test_execute((BaseSequentialStream *)&SD2, &rt_test_suite);
      test_execute((BaseSequentialStream *)&SD2, &oslib_test_suite);
    }
    chThdSleepMilliseconds(1000);
  }
}

