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
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/sensor.h>

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

static const struct pwm_dt_spec *pwms[6] = {&pwm_motor0, &pwm_motor1,
                                            &pwm_motor2, &pwm_motor3,
                                            &pwm_motor4, &pwm_motor5};

static const struct device *const mpu1_dev = DEVICE_DT_GET(DT_NODELABEL(mpu1));
static const struct device *const mpu2_dev = DEVICE_DT_GET(DT_NODELABEL(mpu2));
static const struct device *const bmp180_dev = DEVICE_DT_GET(DT_NODELABEL(bmp180));


int main(void) {
  struct sensor_value p;
  struct sensor_value t;
  struct sensor_value xy1[3];
  struct sensor_value xy2[3];
  int32_t period = PERIOD;
  int32_t duty = 10 * PERIOD / 4 ; //duty de 40% para conexão com escs
  int dpitch = 0;
  int droll = 0;
  double pitch = 0;
  int iroll = 0;
  int ipitch = 0;
  double roll = 0;
  double press = 0;
  int dyaw = 0;
  int ret;
  int32_t duty0 = 0;
  int32_t duty1 = 0;
  int32_t duty2 = 0;
  int32_t duty3 = 0;
  double alt = 0;

  double p1 = 0;
  double r1 = 0;
  double p2 = 0;
  double r2 = 0;


  for (int i = 0; i < NUM_PWMS; i++) {
    if (!pwm_is_ready_dt(pwms[i])) {
      printk("Error: PWM device %s is not ready\n", pwms[i]->dev->name);
      return 0;
    }
  }
  duty = 4 * period / 10; // duty inicial de 40%

  for (int i = 0; i < NUM_PWMS; i++) {
    ret = pwm_set_dt(pwms[i], period, duty);
  }


	k_usleep (10);
    if (!device_is_ready(mpu1_dev)) {
        printk("Erro: Dispositivo MPU1 nao esta pronto.\n");
        return 0;
    }

    // Verifica se o MPU2 está pronto para uso
    if (!device_is_ready(mpu2_dev)) {
        printk("Erro: Dispositivo MPU2 nao esta pronto.\n");
        return 0;
    }

    // Verifica se o BMP180 está pronto para uso
    if (!device_is_ready(bmp180_dev)) {
        printk("Erro: Dispositivo BMP180 nao esta pronto.\n");
        return 0;
    }

 

  int press1 = sensor_sample_fetch(bmp180_dev);

  k_msleep(50);

  press1 = sensor_channel_get(bmp180_dev, SENSOR_CHAN_PRESS,&p);

  int d1 = sensor_sample_fetch(mpu1_dev);
  d1 = sensor_channel_get(mpu1_dev, SENSOR_CHAN_DIE_TEMP, &t);


  double init = sensor_value_to_double(&p);
  double tinit = sensor_value_to_double(&t);

  double temp_kelvin = tinit + 273.15;
    
  double coef = (287.05 * temp_kelvin) / (9.80665 * init);

  //printk("%f , %f , %f",init, tinit, coef);

   while (1) {
    //printk("l");
   
    // 1. Fetch Otimizado: Busca APENAS os canais utilizados via I2C
    sensor_sample_fetch_chan(mpu1_dev, SENSOR_CHAN_ACCEL_XYZ);
    sensor_sample_fetch_chan(mpu2_dev, SENSOR_CHAN_ACCEL_XYZ);
    sensor_sample_fetch_chan(bmp180_dev, SENSOR_CHAN_PRESS);

    // 2. Extrai os dados salvos nos buffers internos dos drivers
    sensor_channel_get(mpu1_dev, SENSOR_CHAN_ACCEL_XYZ, xy1);
    sensor_channel_get(mpu2_dev, SENSOR_CHAN_ACCEL_XYZ, xy2);
    sensor_channel_get(bmp180_dev, SENSOR_CHAN_PRESS, &p);

    // 3. Converte os valores brutos de aceleração e pressão para double
    p1 = sensor_value_to_double(&xy1[0]); // Aceleração X (Sensor 1)
    r1 = sensor_value_to_double(&xy1[1]); // Aceleração Y (Sensor 1)
    
    p2 = sensor_value_to_double(&xy2[0]); // Aceleração X (Sensor 2)
    r2 = sensor_value_to_double(&xy2[1]); // Aceleração Y (Sensor 2)
    
    press = sensor_value_to_double(&p);

    // 4. Cálculo Linear Ultra Rápido da Altitude Relativa
    alt = coef * (init - press);

    // Exibe os dados consolidados de forma limpa no console
   printk("Alt: %.2fm | P1: %.3f | R1: %.3f | P2: %.3f | R2: %.3f\n", alt, p1, r1, p2, r2);
   // printk("l");
   }

  return 0;
}