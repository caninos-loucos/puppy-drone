/*
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file Sample app to demonstrate PWM.
 */

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static const struct pwm_dt_spec pwm_motor0 =
    PWM_DT_SPEC_GET(DT_ALIAS(pwm_motor0));

#define MAX_PERIOD 1200000

int main(void) {
  uint32_t max_period = MAX_PERIOD;
  uint32_t period;
  uint8_t dir = 0U;
  int ret;

  printk("PWM-based blinky\n");

  if (!pwm_is_ready_dt(&pwm_motor0)) {
    printk("Error: PWM device %s is not ready\n", pwm_motor0.dev->name);
    return 0;
  }

  ret = pwm_set_dt(&pwm_motor0, max_period, max_period / 2);

  k_usleep(5000000);

  period = max_period / 2 + max_period / 5;
  while (1) {
    ret = pwm_set_dt(&pwm_motor0, max_period, period);
    if (ret) {
      printk("Error %d: failed to set pulse width\n", ret);
      return 0;
    }
    printk("Using period %d\n", period);

    period = period + (max_period / 20);

    if (period >= (max_period - max_period / 20))
      period = max_period / 2;

    k_usleep(5000000);
  }
  return 0;
}
