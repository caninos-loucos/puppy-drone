/* SPDX-License-Identifier: Apache-2.0
 */

#include <math.h>
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
#define kp 18000
#define ki 200
#define kd 300

/* Saturações dos Motores */
#define MAXIPITCH 2
#define MAXIROLL 2

/* Duty cycle de Conexão com motores */
#define CONNECT_DUTY PERIOD * 0.4f
#define BASE_DUTY PERIOD * 0.44f
#define DUTY_MAX PERIOD * 0.7f
#define DUTY_MIN PERIOD * 0.4f

/* Duty cycles de cada motor */
int32_t motor_duty0 = CONNECT_DUTY;
int32_t motor_duty1 = CONNECT_DUTY;
int32_t motor_duty2 = CONNECT_DUTY;
int32_t motor_duty3 = CONNECT_DUTY;

float duty0 = 0;
float duty1 = 0;
float duty2 = 0;
float duty3 = 0;

float pitch = 0;
float roll = 0;
float ipitch = 0;
float iroll = 0;
float dpitch = 0;
float droll = 0;

float p1 = 0.0f;
float r1 = 0.0f;
float p2 = 0.0f;
float r2 = 0.0f;

struct k_mutex motor_mutex;
struct k_work motor_work;

void update_motors_worker(struct k_work *work) {
  ARG_UNUSED(work);

  pwm_set_dt(pwms[0], PERIOD, motor_duty0);
  pwm_set_dt(pwms[1], PERIOD, motor_duty1);
  pwm_set_dt(pwms[2], PERIOD, motor_duty2);
  pwm_set_dt(pwms[3], PERIOD, motor_duty3);

  if (!gpio_pin_get_dt(&button)) {
    motor_duty0 = CLAMP(duty0, DUTY_MIN, DUTY_MAX);
    motor_duty1 = CLAMP(duty1, DUTY_MIN, DUTY_MAX);
    motor_duty2 = CLAMP(duty2, DUTY_MIN, DUTY_MAX);
    motor_duty3 = CLAMP(duty3, DUTY_MIN, DUTY_MAX);
  } else {
    motor_duty0 = CONNECT_DUTY;
    motor_duty1 = CONNECT_DUTY;
    motor_duty2 = CONNECT_DUTY;
    motor_duty3 = CONNECT_DUTY;

    duty0 = 0.0f;
    duty1 = 0.0f;
    duty2 = 0.0f;
    duty3 = 0.0f;

    ipitch = 0.0f;
    dpitch = 0.0f;
  }

  k_mutex_lock(&motor_mutex, K_FOREVER);
  dpitch = -p1 - p2 + pitch;
  droll = -r1 - r2 + roll;

  duty0 = BASE_DUTY - ((kp * pitch) + (kd * dpitch) + (ki * ipitch)) +
          ((kp * roll) + (kd * droll) + (ki * iroll));
  duty1 = BASE_DUTY + ((kp * pitch) + (kd * dpitch) + (ki * ipitch)) -
          ((kp * roll) + (kd * droll) + (ki * iroll));
  duty2 = BASE_DUTY - ((kp * pitch) + (kd * dpitch) + (ki * ipitch)) -
          ((kp * roll) + (kd * droll) + (ki * iroll));
  duty3 = BASE_DUTY + ((kp * pitch) + (kd * dpitch) + (ki * ipitch)) +
          ((kp * roll) + (kd * droll) + (ki * iroll));

  ipitch += pitch;
  iroll += pitch;

  if (fabsf(ipitch) > 3.5f)
    ipitch = (ipitch / fabsf(ipitch)) * 3.5f;
  if (fabsf(iroll) > 3.5f)
    iroll = (iroll / fabsf(iroll)) * 3.5f;
  k_mutex_unlock(&motor_mutex);
}

void update_motors_timer(struct k_timer *timer) {
  ARG_UNUSED(timer);
  k_work_submit(&motor_work);
}

K_TIMER_DEFINE(motor_timer, &update_motors_timer, NULL);

int main(void) {
  struct sensor_value accel1[3];
  struct sensor_value accel2[3];
  int ret;
  k_mutex_init(&motor_mutex);
  k_work_init(&motor_work, &update_motors_worker);

  if (!device_is_ready(imu1)) {
    printf("IMU 1 not Ready!\n");
    return 0;
  }

  if (!device_is_ready(imu2)) {
    printf("IMU 2 not Ready\n");
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

  if (gpio_pin_get_dt(&button)) {
    while (gpio_pin_get_dt(&button)) {
      printk("Desligue o botão e ligue de novo para iniciar!\n");
      k_usleep(500000);
    }
  }

  k_timer_start(&motor_timer, K_MSEC(1), K_MSEC(1));

  while (1) {
    sensor_sample_fetch(imu1);
    sensor_channel_get(imu1, SENSOR_CHAN_ACCEL_XYZ, accel1);
    sensor_sample_fetch(imu2);
    sensor_channel_get(imu2, SENSOR_CHAN_ACCEL_XYZ, accel2);

    p1 = -sensor_value_to_float(&accel1[0]);
    p2 = -sensor_value_to_float(&accel2[0]);
    r1 = -sensor_value_to_float(&accel1[1]);
    r2 = -sensor_value_to_float(&accel2[1]);

    k_mutex_lock(&motor_mutex, K_FOREVER);

    pitch = (p1 + p2) / 2;
    roll = (r1 + r2) / 2;

    if (fabsf(pitch) > 3.5f)
      pitch = (pitch / fabsf(pitch)) * 3.5f;
    if (fabsf(roll) > 3.5f)
      roll = (roll / fabsf(roll)) * 3.5f;

    k_mutex_unlock(&motor_mutex);
  }

  return 0;
}