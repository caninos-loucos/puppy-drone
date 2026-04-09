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

#define PERIOD 2500000

int main(void) {
  uint32_t period = PERIOD;
  uint32_t duty;
  uint8_t dir = 0U;
  int ret;

  printk("PWM-based blinky\n");

  if (!pwm_is_ready_dt(&pwm_motor0)) {
    printk("Error: PWM device %s is not ready\n", pwm_motor0.dev->name);
    return 0;
  }
  duty = 4 * period / 10; //duty inicial de 40%
  ret = pwm_set_dt(&pwm_motor0, period, duty);
      printk("conectando esc");


  k_usleep(10000000);

  while (1) {
    ret = pwm_set_dt(&pwm_motor0, period, duty);
    if (ret) {
      printk("Error %d: failed to set pulse width\n", ret);
      return 0;
    }

    duty = 6 * period / 10; //duty de 60%
    ret = pwm_set_dt(&pwm_motor0, period, duty);
    printk("Using duty %d\n", duty);
    k_usleep(3000000);

    duty = 4 * period / 10; //duty de 40% (parado)
    ret = pwm_set_dt(&pwm_motor0, period, duty);
    printk("Using duty %d\n", duty);
    k_usleep(10000000);
  }
  return 0;
}

