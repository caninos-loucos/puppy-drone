#ifndef PID_H
#define PID_H

#define RAD_TO_DEG (180.0f / 3.14159f)

#define DUTY_REF 0.4f

void controller(float ax, float ay, float az, float vx, float vy,
                float *roll_duty_P, float *roll_duty_N, float *pitch_duty_P, float *pitch_duty_N);
#endif 