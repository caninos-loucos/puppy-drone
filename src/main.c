/*
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file Sample app to demonstrate PWM.
 */

#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static const struct pwm_dt_spec pwm_motor0 =
    PWM_DT_SPEC_GET(DT_ALIAS(pwm_motor0));

#define PERIOD 1250000

int main(void) {
  int32_t period = PERIOD;
  int32_t duty;
  int dir = 0U;
  int ret;

  if (!pwm_is_ready_dt(&pwm_motor0)) {
    printk("Error: PWM device %s is not ready\n", pwm_motor0.dev->name);
    return 0;
  }

  duty = 5 * period / 10; // duty inicial de 50%
  ret = pwm_set_dt(&pwm_motor0, period, duty);
  printk("Connecting ESC\n");
  dir = 1;
  k_usleep(5000000);

  while (1) {
    ret = pwm_set_dt(&pwm_motor0, period, duty);
    if (ret) {
      printk("Error %d: failed to set pulse width\n", ret);
      return 0;
    }

    duty += (int32_t)dir * period / 200;
    if ((duty >= 7 * period / 10) || (duty <= 5 * period / 10)) {
      dir = -dir;
    }

    ret = pwm_set_dt(&pwm_motor0, period, duty);
    printk("Using duty %d ns (%.1f)\n", duty, (double)(duty * 100) / period);
    k_usleep(1000000);
  }
  return 0;
}
