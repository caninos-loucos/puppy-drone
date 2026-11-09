#include "pid.h"
#include "drone_model.h"
#include <math.h>
/*
 * ax, ay, az -> acelerômetro
 * vx, vy     -> giroscópio em graus/s
 */
void controller(float ax, float ay, float az, float vx, float vy,
                float *roll_duty_P, float *roll_duty_N, float *pitch_duty_P, float *pitch_duty_N) {
  /* Calcula orientação assumindo que a aceleração não é maiora que a gravidade
   */
  float roll = atan2f(ay, az) * RAD_TO_DEG;

  float pitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD_TO_DEG;

  /* Referências */
  float roll_ref = 0.0f;
  float pitch_ref = 0.0f;

  /* Erros */
  float roll_error = roll_ref - roll;

  float pitch_error = pitch_ref - pitch;

  /* Controle PD */
  *roll_duty_P = DUTY_REF + Kp_roll * roll_error - Kd_roll * vx;
  *roll_duty_N = DUTY_REF - Kp_roll * roll_error + Kd_roll * vx;

  *pitch_duty_P = DUTY_REF + Kp_pitch * pitch_error - Kd_pitch * vy;
  *pitch_duty_N = DUTY_REF - Kp_pitch * pitch_error + Kd_pitch * vy;

  /* Saturação */
  if (*roll_duty_P > 1.0f)
    *roll_duty_P = 1.0f;

  if (*roll_duty_P < 0.0f)
    *roll_duty_P = 0.0f;

  if (*pitch_duty_P > 1.0f)
    *pitch_duty_P = 1.0f;

  if (*pitch_duty_P < 0.0f)
    *pitch_duty_P = 0.0f;

  if (*roll_duty_N > 1.0f)
    *roll_duty_N = 1.0f;

  if (*roll_duty_N < 0.0f)
    *roll_duty_N = 0.0f;

  if (*pitch_duty_N > 1.0f)
    *pitch_duty_N = 1.0f;

  if (*pitch_duty_N < 0.0f)
    *pitch_duty_N = 0.0f;
}