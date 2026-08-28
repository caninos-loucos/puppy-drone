/* SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

/* Obter dispositivos da Devicetree */

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

static const struct gpio_dt_spec button =
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(button0), gpios, {0});

#define PERIOD 2500000
#define NUM_PWMS 4

static const struct pwm_dt_spec *pwms[NUM_PWMS] = {&pwm_motor0, &pwm_motor1,
                                                   &pwm_motor2, &pwm_motor3};

/* Constantes de Controle */
#define kpp 60000
#define kpr 60000
#define kdp 200
#define kdr 200
#define kip 300
#define kir 300

/* Saturações dos Motores */
#define MAXIPITCH 2
#define MAXIROLL 2

/* Duty cycle de Conexão com motores */
#define CONNECT_DUTY 4 * PERIOD / 10
#define BASE_DUTY 46 * PERIOD / 100
#define DUTY_MAX 7 * PERIOD / 10
#define DUTY_MIN 4 * PERIOD / 10

/* Duty cycles de cada motor */
int32_t duty0 = 0;
int32_t duty1 = 0;
int32_t duty2 = 0;
int32_t duty3 = 0;
float pitch = 0;
float roll = 0;
float ipitch = 0;
float iroll = 0;
float dpitch = 0;
float droll = 0;
float yaw = 0;
float p1 = 0;
float r1 = 0;
float p2 = 0;
float r2 = 0;

void update_motors(struct k_timer *timer) {
  ARG_UNUSED(timer);

  if (!gpio_pin_get_dt(&button) && (abs(roll) < 3.5) && (abs(pitch) < 3.5)) {
    pwm_set_dt(pwms[0], PERIOD, duty0);
    pwm_set_dt(pwms[1], PERIOD, duty1);
    pwm_set_dt(pwms[2], PERIOD, duty2);
    pwm_set_dt(pwms[3], PERIOD, duty3);

  } else {
    pwm_set_dt(pwms[0], PERIOD, CONNECT_DUTY);
    pwm_set_dt(pwms[1], PERIOD, CONNECT_DUTY);
    pwm_set_dt(pwms[2], PERIOD, CONNECT_DUTY);
    pwm_set_dt(pwms[3], PERIOD, CONNECT_DUTY);
    yaw = 0;
    //printk("coords: P = %f \tR = %f \tbutão = %d \t", pitch, roll,
           //gpio_pin_get_dt(&button));
   // printk("dutys: 1 = %d \t2 = %d \t3 = %d \t4 = %d  \tcerto = %d \n", duty0,
          // duty1, duty2, duty3, CONNECT_DUTY);
  }
}

K_TIMER_DEFINE(motor_timer, &update_motors, NULL);

int main(void) {
  struct sensor_value accel1[3];
  struct sensor_value accel2[3];
  int ret;

  if (!device_is_ready(imu1)) {
    printf("1 não foi\n");
    return 0;
  }

  if (!device_is_ready(imu2)) {
    printf("2 não foi\n");
    return 0;
  }

  if (!gpio_is_ready_dt(&button)) {
    printk("Error: Button is not Ready\n");
    return 0;
  }

  for (int i = 0; i < NUM_PWMS; i++) {
    if (!pwm_is_ready_dt(pwms[i])) {
      printk("Error: PWM device %s is not ready\n", pwms[i]->dev->name);
      return 0;
    }
  }

  gpio_pin_configure_dt(&button, GPIO_INPUT);

  for (int i = 0; i < NUM_PWMS; i++) {
    ret = pwm_set_dt(pwms[i], PERIOD, CONNECT_DUTY);
  }

  printk("Connecting ESCs, wait 5 seconds for setup\n");

  k_usleep(5000000);

  if (gpio_pin_get_dt(
          &button)) { // se botão ligado, avisar antes de entrar no loop
    while (gpio_pin_get_dt(&button)) {
      printk("Desligue o botão e ligue de novo para iniciar!\n");
      k_usleep(500000);
    }
  }

  k_timer_start(&motor_timer, K_USEC(2000), K_USEC(2000));

  while (1) {

    sensor_sample_fetch(imu1);
    sensor_channel_get(imu1, SENSOR_CHAN_ACCEL_XYZ, accel1);
    sensor_sample_fetch(imu2);
    sensor_channel_get(imu2, SENSOR_CHAN_ACCEL_XYZ, accel2);

    p1 = -sensor_value_to_float(&accel1[0]) / 2;
    p2 = -sensor_value_to_float(&accel2[0]) / 2;
    r1 = -sensor_value_to_float(&accel1[1]) / 2;
    r2 = -sensor_value_to_float(&accel2[1]) / 2;

    /* Disable IRQs while calculating the duty and roll/yaw values so that
     * intermediate values are not sent to the motors */
    uint32_t key = irq_lock();

    dpitch = -p1 - p2 + pitch;
    droll = -r1 - r2 + roll;

    pitch = p1 + p2;
    roll = r1 + r2;

    iroll = CLAMP(iroll, -MAXIROLL, MAXIROLL);
    ipitch = CLAMP(ipitch, -MAXIPITCH, MAXIPITCH);

    duty0 = (uint32_t)BASE_DUTY - ((kpp * pitch) + (kdp * dpitch) + (kip * ipitch)) +
            ((kpr * roll) + (kdr * droll) + (kir * iroll));
    duty1 = (uint32_t)BASE_DUTY + ((kpp * pitch) + (kdp * dpitch) + (kip * ipitch)) -
            ((kpr * roll) + (kdr * droll) + (kir * iroll));
    duty2 = (uint32_t)BASE_DUTY - ((kpp * pitch) + (kdp * dpitch) + (kip * ipitch)) -
            ((kpr * roll) + (kdr * droll) + (kir * iroll));
    duty3 =(uint32_t) BASE_DUTY + ((kpp * pitch) + (kdp * dpitch) + (kip * ipitch)) +
            ((kpr * roll) + (kdr * droll) + (kir * iroll));

    duty0 = CLAMP(duty0, (uint32_t)DUTY_MIN, (uint32_t)DUTY_MAX);

    duty1 = CLAMP(duty1, (uint32_t)DUTY_MIN, (uint32_t)DUTY_MAX);

    duty2 = CLAMP(duty2, (uint32_t)DUTY_MIN, (uint32_t)DUTY_MAX);

    duty3 = CLAMP(duty3, (uint32_t)DUTY_MIN, (uint32_t)DUTY_MAX);

    irq_unlock(key);

    ipitch += pitch;
    iroll += pitch;
  }

  return 0;
}