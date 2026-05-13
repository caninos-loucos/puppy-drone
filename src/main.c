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

static const struct pwm_dt_spec pwm_motor1 =
    PWM_DT_SPEC_GET(DT_ALIAS(pwm_motor1));

static const struct pwm_dt_spec pwm_motor2 =
    PWM_DT_SPEC_GET(DT_ALIAS(pwm_motor2));

static const struct pwm_dt_spec pwm_motor3 =
    PWM_DT_SPEC_GET(DT_ALIAS(pwm_motor3));

static const struct pwm_dt_spec pwm_motor4 =
    PWM_DT_SPEC_GET(DT_ALIAS(pwm_motor4));

static const struct pwm_dt_spec pwm_motor5 =
    PWM_DT_SPEC_GET(DT_ALIAS(pwm_motor5));

#define PERIOD 2500000
#define NUM_PWMS 6

static const struct pwm_dt_spec* pwms[6] = {
  &pwm_motor0,
  &pwm_motor1,
  &pwm_motor2,
  &pwm_motor3,
  &pwm_motor4,
  &pwm_motor5
};

int main(void) {
  int32_t period = PERIOD;
  int32_t duty;
  int dir = 0U;
  int ret;

  for(int i=0; i < NUM_PWMS; i++)
  {
    if (!pwm_is_ready_dt(pwms[i])) {
      printk("Error: PWM device %s is not ready\n", pwms[i]->dev->name);
      return 0;
    }
  }
  duty = 5 * period / 10; // duty inicial de 50%

  for(int i = 0; i < NUM_PWMS; i++)
  {
    ret = pwm_set_dt(pwms[i], period, duty);
  }

  printk("Connecting ESC\n");
  dir = 1;
  k_usleep(5000000);

  do
  {
    duty += (int32_t)dir * period / 200;
    if ((duty >= 7 * period / 10)) {
      dir = -dir;
    }

    for(int i = 0; i < NUM_PWMS; i++)
    {
      ret = pwm_set_dt(pwms[i], period, duty);
      if (ret) {
        printk("Error %d: failed to set pulse width\n", ret);
        return 0;
      }
    }

    printk("Using duty %d ns (%.1f)\n", duty, (double)(duty * 100) / period);
    k_usleep(2000000);
  } while ((duty > 5 * period / 10));

  return 0;
}
