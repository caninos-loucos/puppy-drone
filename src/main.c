/* SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file Sample app to demonstrate PWM.
 */

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static const struct device *const imu1 = DEVICE_DT_GET(DT_NODELABEL(mpu1));

static const struct device *const imu2 = DEVICE_DT_GET(DT_NODELABEL(mpu2));


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
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(button0), gpios, {0});

#define PERIOD 2500000
#define NUM_PWMS 6

  // CONSTANTES DE CONTROLE
#define kpp 50000
#define kpr 50000
#define kdp 500
#define kdr 500
#define kip 100
#define kir 100
#define MAXIPITCH 2
#define MAXIROLL 2

  // saturações motores



static const struct pwm_dt_spec *pwms[6] = {&pwm_motor0, &pwm_motor1,
                                            &pwm_motor2, &pwm_motor3,
                                            &pwm_motor4, &pwm_motor5};

int main(void) {


	if (!device_is_ready(imu1)) {
		printf("1 não foi");
		return 0;
	}

  if (!device_is_ready(imu2)) {
		printf("2 não foi");
		return 0;
	}

  struct sensor_value accel1[3];
  struct sensor_value accel2[3];
  int32_t period = PERIOD;
  int32_t duty = 4 * PERIOD / 10 ; //duty de 40% para conexão com escs
  int32_t base = 44 * PERIOD / 100;
  double pitch = 0;
  double roll = 0;
  double ipitch = 0;
  double iroll = 0;
  double dpitch = 0;
  double droll = 0;
  double yaw = 0;
  double p1 = 0;
  double r1 = 0;
  double p2 = 0;
  double r2 = 0;

  int32_t dutymax = 7 * PERIOD /10;
  int32_t dutymin = 4 * PERIOD /10;

  int ret;
  int32_t duty0 = 0;
  int32_t duty1 = 0;
  int32_t duty2 = 0;
  int32_t duty3 = 0;



  if (!gpio_is_ready_dt(&button)) {
    printk("Error: Button is not Ready\n");
    return 0;
  }

  gpio_pin_configure_dt(&button, GPIO_INPUT);

  for (int i = 0; i < NUM_PWMS; i++) {
    if (!pwm_is_ready_dt(pwms[i])) {
      printk("Error: PWM device %s is not ready\n", pwms[i]->dev->name);
      return 0;
    }
  }

  for (int i = 0; i < NUM_PWMS; i++) {
    ret = pwm_set_dt(pwms[i], period, duty);
  }

  printk("Connecting ESCs, wait 5 seconds for setup\n");

   k_usleep(5000000);

  if (gpio_pin_get_dt(&button)) { // se botão ligado, avisar antes de entrar no loop
    while (gpio_pin_get_dt(&button)){
      printk ("Desligue o botão e ligue de novo para iniciar");
      k_usleep (500000);
    }
  }

  while (1) {

    //lendo sensor

    sensor_sample_fetch(imu1);
    sensor_channel_get(imu1, SENSOR_CHAN_ACCEL_XYZ, accel1);
    sensor_sample_fetch(imu2);
    sensor_channel_get(imu2, SENSOR_CHAN_ACCEL_XYZ, accel2);

    //se quiser printar accell
    //printk("Accel: X=%f, Y=%f, Z=%f m/s^2\n",sensor_value_to_double(&accel[0]),sensor_value_to_double(&accel[1]),sensor_value_to_double(&accel[2]));

    p1 = - sensor_value_to_double(&accel1[0])/2;
    p2 = - sensor_value_to_double(&accel2[0])/2;
    r1 = - sensor_value_to_double(&accel1[1])/2;
    r2 = - sensor_value_to_double(&accel2[1])/2;

    //atualizando o controle
    dpitch =  - p1 - p2 + pitch;
    droll =  - r1 - r2 + roll;

    pitch = p1 + p2;
    roll =  r1 + r2;

    if (iroll > MAXIROLL){
      iroll = MAXIROLL;
    }

    if (iroll < MAXIROLL){
      iroll =  - MAXIROLL;
    }

    if (ipitch > MAXIPITCH){
      ipitch = MAXIPITCH;
    }

    if (ipitch < MAXIPITCH){
      ipitch = - MAXIPITCH;
    }
    

    //CONTAS em NANOSEGUNDOS (2500000 é 100%)
    duty0 = base - ((kpp * pitch) + (kdp * dpitch) + (kip * ipitch)) + ((kpr * roll) + (kdr * droll) + (kir * iroll));
    duty1 = base + ((kpp * pitch) + (kdp * dpitch) + (kip * ipitch)) - ((kpr * roll) + (kdr * droll) + (kir * iroll));
    duty2 = base - ((kpp * pitch) + (kdp * dpitch) + (kip * ipitch)) - ((kpr * roll) + (kdr * droll) + (kir * iroll));
    duty3 = base + ((kpp * pitch) + (kdp * dpitch) + (kip * ipitch)) + ((kpr * roll) + (kdr * droll) + (kir * iroll));

      //MOTOR1:
      if (duty0 > dutymax){
        duty0 = dutymax;
      } else if (duty0 < dutymin){
        duty0 = dutymin;
      } else {
        ipitch += pitch;
        iroll += pitch;
      }


      //MOTOR2:
      if (duty1 > dutymax){
        duty1 = dutymax;
      } else if (duty1 < dutymin){
        duty1 = dutymin;
      } else {
        ipitch += pitch;
        iroll += pitch;
      }

      //MOTOR3:
      if (duty2 > dutymax){
        duty2 = dutymax;
      } else if (duty2 < dutymin){
        duty2 = dutymin;
      } else {
        ipitch += pitch;
        iroll += pitch;
      }

      //MOTOR4:
      if (duty3 > dutymax){
        duty3 = dutymax;
      } else if (duty3 < dutymin){
        duty3 = dutymin;
      } else {
        ipitch += pitch;
        iroll += pitch;
      }

    if (!gpio_pin_get_dt(&button) && (abs(roll) < 3) && (abs(pitch) < 3)) { 
        pwm_set_dt(pwms[0], period, duty0);
        pwm_set_dt(pwms[1], period, duty1);
        pwm_set_dt(pwms[2], period, duty2);
        pwm_set_dt(pwms[3], period, duty3);
        printk("oi\n");
        k_usleep(4000);


    } else { // sem botão, deligar PWM
        pwm_set_dt(pwms[0], period, duty);
        pwm_set_dt(pwms[1], period, duty);
        pwm_set_dt(pwms[2], period, duty);
        pwm_set_dt(pwms[3], period, duty);
        yaw = 0;
        printk ("coords: P = %f R = %f butão = %d \n", pitch, roll, gpio_pin_get_dt(&button));
        printk ("dutys: 1 = %d 2 = %d 3 = %d 4 = %d  certo = %d \n", (duty0 * 100)/ PERIOD , (duty1 * 100)/ PERIOD, (duty2 * 100)/ PERIOD, (duty3 * 100)/ PERIOD , (duty * 100)/ PERIOD);
    }
  }

  return 0;
}