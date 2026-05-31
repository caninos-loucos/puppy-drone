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

static const struct device *const mpu6050 = DEVICE_DT_GET_ONE(invensense_mpu6050);

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

  // CONSTANTES DE CONTROLE (SÓ PROPORCIONAL POR ENQUANTO) (exceto dy, que é integral)
#define kpp 0
#define kpr 0
#define kiy 0
#define base 30*PERIOD/100

  // saturações motores

#define dutymax  8 * PERIOD /10 //saturação do motor
#define dutymin  4 * PERIOD /10 //minimo do motor em voo

static const struct pwm_dt_spec *pwms[6] = {&pwm_motor0, &pwm_motor1,
                                            &pwm_motor2, &pwm_motor3,
                                            &pwm_motor4, &pwm_motor5};

int main(void) {

  printk("Hello\n");

	if (!device_is_ready(mpu6050)) {
		printf("Device %s is not ready\n", mpu6050->name);
		return 0;
	}

  struct sensor_value accel[3];
  struct sensor_value gyro[3];
  int32_t period = PERIOD;
  int32_t duty = 10 * PERIOD / 4 ; //duty de 40% para conexão com escs
  int pitch = 0;
  int roll = 0;
  int dyaw = 0;
  int ret;
  int32_t duty0 = 0;
  int32_t duty1 = 0;
  int32_t duty2 = 0;
  int32_t duty3 = 0;
  int yaw = 0;



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

    sensor_sample_fetch(mpu6050);
    sensor_channel_get(mpu6050, SENSOR_CHAN_ACCEL_XYZ, accel);
    sensor_channel_get(mpu6050, SENSOR_CHAN_GYRO_XYZ, gyro);

    //se quiser printar accell
    printk("Accel: X=%f, Y=%f, Z=%f m/s^2\n",sensor_value_to_double(&accel[0]),sensor_value_to_double(&accel[1]),sensor_value_to_double(&accel[2]));
    // se quiser printar gyro
    printk("Gyro:  X=%f, Y=%f, Z=%f rad/s\n", sensor_value_to_double(&gyro[0]), sensor_value_to_double(&gyro[1]), sensor_value_to_double(&gyro[2]));

    pitch = - sensor_value_to_double(&accel[0]);
    roll =  - sensor_value_to_double(&accel[1]);
    dyaw =  sensor_value_to_double(&gyro[2]);
    yaw = yaw + dyaw ;


    //CONTAS em NANOSEGUNDOS (2500000 é 100%)
    duty0 = base - (kpp * pitch) + (kpr * roll) - (kiy * yaw);
    duty1 = base + (kpp * pitch) - (kpr * roll) - (kiy * yaw);
    duty2 = base - (kpp * pitch) - (kpr * roll) + (kiy * yaw);
    duty3 = base + (kpp * pitch) + (kpr * roll) + (kiy * yaw);



    if (gpio_pin_get_dt(&button)) { // se botão ligado, ativar motores
      //MOTOR1:
      if (duty0 > dutymax){
        ret = pwm_set_dt(pwms[0], period, dutymax);
      } else if (duty0 < dutymin){
        ret = pwm_set_dt(pwms[0], period, dutymin);
      } else {
        ret = pwm_set_dt(pwms[0], period, duty0);
      }


      //MOTOR2:
      if (duty1 > dutymax){
        ret = pwm_set_dt(pwms[1], period, dutymax);
      } else if (duty1 < dutymin){
        ret = pwm_set_dt(pwms[1], period, dutymin);
      } else {
        ret = pwm_set_dt(pwms[1], period, duty1);
      }


      //MOTOR3:
      if (duty2 > dutymax){
        ret = pwm_set_dt(pwms[2], period, dutymax);
      } else if (duty2 < dutymin){
        ret = pwm_set_dt(pwms[2], period, dutymin);
      } else {
        ret = pwm_set_dt(pwms[2], period, duty2);
      }


      //MOTOR4:
      if (duty3 > dutymax){
        ret = pwm_set_dt(pwms[3], period, dutymax);
      } else if (duty3 < dutymin){
        ret = pwm_set_dt(pwms[3], period, dutymin);
      } else {
        ret = pwm_set_dt(pwms[3], period, duty3);
      }


    } else { // sem botão, deligar PWM
        ret = pwm_set_dt(pwms[0], period, duty);
        ret = pwm_set_dt(pwms[1], period, duty);
        ret = pwm_set_dt(pwms[2], period, duty);
        ret = pwm_set_dt(pwms[3], period, duty);
        yaw = 0;
        printk ("coords: P = %d R = %d dY = %d\n", pitch, roll, dyaw);
        printk ("dutys: 1 = %d 2 = %d 3 = %d 4 = %d \n", (duty0 * 100)/ PERIOD , (duty1 * 100)/ PERIOD, (duty2 * 100)/ PERIOD, (duty3 * 100)/ PERIOD);
    }
    k_usleep(1);
  }

  return 0;
}