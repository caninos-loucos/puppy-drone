#ifndef DRONE_MODEL_H
#define DRONE_MODEL_H

#include <zephyr/drivers/sensor.h>

#define CONNECT_DUTY PERIOD * 0.4f
#define BASE_DUTY PERIOD * 0.44f
#define DUTY_MAX PERIOD * 0.7f
#define DUTY_MIN PERIOD * 0.4f

#define Kp_roll  0.005f
#define Kd_roll  0.001f

#define Kp_pitch 0.005f
#define Kd_pitch 0.001f

struct drone_model {
        float duty_x_P;
        float duty_x_N;
        float duty_y_P;
        float duty_y_N;
        struct sensor_value acc[3];
        struct sensor_value gyro[3];
        uint32_t last_sample_timestamp;
};

#endif