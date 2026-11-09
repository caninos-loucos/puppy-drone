/* SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include "drone_model.h"
#include "pid.h"

/* Obter dispositivos da Devicetree */

static const struct device *const imu1 = DEVICE_DT_GET(DT_NODELABEL(mpu1));

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
struct k_mutex motor_mutex;
struct k_work motor_work;

struct drone_model drone;
static struct sensor_trigger data_trigger;

void imu_trig_callback(const struct device *dev,
                       const struct sensor_trigger *trig) {
  if (trig->type == SENSOR_TRIG_DATA_READY) {
    int rc = sensor_sample_fetch_chan(dev, trig->chan);

    if (rc < 0) {
      printf("sample fetch failed: %d\n", rc);
      printf("cancelling trigger due to failure: %d\n", rc);
      (void)sensor_trigger_set(dev, trig, NULL);
    }
    sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, drone.acc);
    sensor_channel_get(dev, SENSOR_CHAN_GYRO_XYZ, drone.gyro);
    drone.last_sample_timestamp = k_uptime_get();
  }
}

void update_motors_worker(struct k_work *work) {
  ARG_UNUSED(work);

  pwm_set_dt(pwms[0], PERIOD, drone.duty_x_P * PERIOD);
  pwm_set_dt(pwms[1], PERIOD, drone.duty_y_P * PERIOD);
  pwm_set_dt(pwms[2], PERIOD, drone.duty_x_N * PERIOD);
  pwm_set_dt(pwms[3], PERIOD, drone.duty_y_N * PERIOD);

  k_mutex_lock(&motor_mutex, K_FOREVER);

  controller(sensor_value_to_float(&drone.acc[0]),
             sensor_value_to_float(&drone.acc[1]),
             sensor_value_to_float(&drone.acc[2]),
             sensor_value_to_float(&drone.gyro[0]),
             sensor_value_to_float(&drone.gyro[1]),
             &drone.duty_x_P,
             &drone.duty_x_N,
             &drone.duty_y_P,
             &drone.duty_y_N);

  k_mutex_unlock(&motor_mutex);
}

void update_motors_timer(struct k_timer *timer) {
  ARG_UNUSED(timer);
  k_work_submit(&motor_work);
}

K_TIMER_DEFINE(motor_timer, &update_motors_timer, NULL);

int main(void) {
  int ret;
  k_mutex_init(&motor_mutex);
  k_work_init(&motor_work, &update_motors_worker);

  if (!device_is_ready(imu1)) {
    printf("IMU 1 not Ready!\n");
    return 0;
  }

  for (int i = 0; i < NUM_PWMS; i++) {
    if (!pwm_is_ready_dt(pwms[i])) {
      printk("Error: PWM device %s is not ready\n", pwms[i]->dev->name);
      return 0;
    }
  }

  data_trigger = (struct sensor_trigger){
      .type = SENSOR_TRIG_DATA_READY,
      .chan = SENSOR_CHAN_ALL,
  };
  if (sensor_trigger_set(imu1, &data_trigger, &imu_trig_callback) < 0) {
    printf("Cannot configure data trigger!!!\n");
    return 0;
  }

  gpio_pin_configure_dt(&button, GPIO_INPUT);

  for (int i = 0; i < NUM_PWMS; i++) {
    ret = pwm_set_dt(pwms[i], PERIOD, CONNECT_DUTY);
  }

  printk("Connecting ESCs, wait 5 seconds for setup\n");

  k_sleep(K_SECONDS(5));

  k_timer_start(&motor_timer, K_USEC(2500), K_USEC(2500));

  return 0;
}