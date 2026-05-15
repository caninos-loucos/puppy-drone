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

static const struct gpio_dt_spec button =
    GPIO_DT_SPEC_GET(DT_ALIAS(button0));

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
  int up = 0;
  int down = 0;
  gpio_configure_dt(&button, GPIO_INPUT);


  for(int i=0; i < NUM_PWMS; i++)
  {
    if (!pwm_is_ready_dt(pwms[i])) {
      printk("Error: PWM device %s is not ready\n", pwms[i]->dev->name);
      return 0;
    }
  }
  duty = 4 * period / 10; // duty inicial de 40%

  for(int i = 0; i < NUM_PWMS; i++)
  {
    ret = pwm_set_dt(pwms[i], period, duty);
  }

  printk("Connecting ESCs\n");
  dir = 1;
  k_usleep(5000000);

  while (1){
    if (gpio_pin_get_dt(&button)){ //se botão ligado, alternar os motores
      if (up == 4) { // se os 4 motores estiverem ligados, desligar um por um
      ret = pwm_set_dt(pwms[down], period, duty);
      down = down + 1;
        if (down == 4){ // se os 4 motores estiverem desligados, resetar os contadores
          up = 0;
          down =  0;
        }
      } else { // se os motores estiverem desligados, ligar um por um
        ret = pwm_set_dt(pwms[up], period, 1.5*duty); 
        up = up + 1;
      } 
      k_usleep(1000000); 
        }
      
      k_usleep(100);
  }

  return 0;
}
