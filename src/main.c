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
#include <zephyr/drivers/uart.h>

static const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart0));

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
#define kpp 42000
#define kpr 42000
#define kdp 8000
#define kdr 8000
#define kip 250
#define kir 250
#define MAXIPITCH 150
#define MAXIROLL 150

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
  struct sensor_value gyro1[3];
  struct sensor_value accel1[3];
  struct sensor_value accel2[3];
  int32_t period = PERIOD;
  int32_t duty = 4 * PERIOD / 10 ; //duty de 40% para conexão com escs
  int32_t base = 47 * PERIOD / 100;

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
  uint8_t c = 0;
  uint8_t x = 0;
  uint8_t y = 0;
  int32_t dutymax = 7 * PERIOD /10;
  int32_t dutymin = 41 * PERIOD /100;
  double oldpitch = 0;
  double oldroll = 0;
  double calibp = 0;
  double calibr = 0;

  double rawpitch =0;
  double rawroll = 0;

  int ret;
  int32_t duty0 = 0;
  int32_t duty1 = 0;
  int32_t duty2 = 0;
  int32_t duty3 = 0;
  int flagbyte = 0;
  int flagcair = 0;
  //offset para controle
  double offp = 0;
  double offr = 0;

  double entrp = 0; 
  double entrr = 0;



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

  if (!gpio_pin_get_dt(&button)) { // se botão ligado, avisar antes de entrar no loop
    while (!gpio_pin_get_dt(&button)){
      printk ("Desligue o botão e ligue de novo para iniciar");
      k_usleep (500000);
    }
  }

  while (1) {
   if (uart_poll_in(uart_dev, &c) == 0) {
        if (c == 240){
          calibp = rawpitch/2;
          calibr = rawroll/2;
          ipitch = 0;
          iroll = 0;
          printk ("CALIBRADO %f,%f\n", calibp, calibr);

        }
        if (c == 255){
          flagbyte = 1;
        }
        else if (c == 0){
          flagbyte = 0;
          entrp = 0;
          entrr = 0;
        }
        else {
          y = c / 16;
          x = c % 16;
          //printk ("r %d e %d \n", x, y);
          if (flagbyte)
          {
            entrp = (x-8) * 0.08;
            entrr = (y-8) * 0.08;
          }
        
        } 
   }else {
           k_usleep(500);
   }

   if (entrp > offp){
    offp = offp + 0.01;
   }

      if (entrp < offp){
    offp = offp - 0.01;
   }

      if (entrr > offr){
    offr = offr + 0.01;
   }

      if (entrr < offr){
    offr = offr - 0.01;
   }

    if (flagcair){
      printk ("b : %d offs: %f e %f ",flagbyte,offp,offr);
    }

    //lendo sensor

    sensor_sample_fetch(imu1);
    sensor_channel_get(imu1, SENSOR_CHAN_ACCEL_XYZ, accel1);
    sensor_channel_get(imu1, SENSOR_CHAN_GYRO_XYZ, gyro1);
    sensor_sample_fetch(imu2);
    sensor_channel_get(imu2, SENSOR_CHAN_ACCEL_XYZ, accel2);

    //se quiser printar accell
    //printk("Accel: X=%f, Y=%f, Z=%f m/s^2\n",sensor_value_to_double(&accel[0]),sensor_value_to_double(&accel[1]),sensor_value_to_double(&accel[2]));

    p1 = - sensor_value_to_double(&accel1[0])/2;
    p2 = - sensor_value_to_double(&accel2[0])/2;
    r1 = - sensor_value_to_double(&accel1[1])/2;
    r2 = - sensor_value_to_double(&accel2[1])/2;
    rawpitch = p1 + p2;
    rawroll =  r1 + r2; 
    p1 = p1 - calibp;
    p2 = p2 - calibp;
    r1 = r1 - calibr;
    r2 = r2 - calibr;

    if (abs(p1 + p2)> 3 || abs(r1 + r2)> 3){
      flagcair = 1;
    } else {
      flagcair = 0;
    }



    //atualizando o controle
    dpitch =  sensor_value_to_double(&gyro1[1]);
    droll =  sensor_value_to_double(&gyro1[0]);

    //pitch =  (p1 + p2 + oldpitch)/2 + offp;
    //roll =  (r1 + r2 + oldroll)/2 + offr;
    //oldpitch = p1 + p2;
    //oldroll = r1 + r2;
    pitch = p1 + p2 + offp;
    roll = r1 + r2 + offr;

    ipitch += pitch + offp;
    iroll += roll + offr;

    //CONTAS em NANOSEGUNDOS (2500000 é 100%)
    duty0 = base - ((kpp * pitch) - (kdp * dpitch) + (kip * ipitch)) + ((kpr * roll) - (kdr * droll) + (kir * iroll));
    duty1 = base + ((kpp * pitch) - (kdp * dpitch) + (kip * ipitch)) - ((kpr * roll) - (kdr * droll) + (kir * iroll));
    duty2 = base - ((kpp * pitch) - (kdp * dpitch) + (kip * ipitch)) - ((kpr * roll) - (kdr * droll) + (kir * iroll));
    duty3 = base + ((kpp * pitch) - (kdp * dpitch) + (kip * ipitch)) + ((kpr * roll) - (kdr * droll) + (kir * iroll));

      //MOTOR1:
      if (duty0 > dutymax){
                printk("satura\n");
        duty0 = dutymax;
      } else if (duty0 < dutymin){
        duty0 = dutymin;
      } 


      //MOTOR2:
      if (duty1 > dutymax){
        printk("satura\n");
        duty1 = dutymax;
      } else if (duty1 < dutymin){
        duty1 = dutymin;
      } 

      //MOTOR3:
      if (duty2 > dutymax){
                printk("satura\n");
        duty2 = dutymax;
      } else if (duty2 < dutymin){
        duty2 = dutymin;
      } 

      //MOTOR4:
      if (duty3 > dutymax){
                printk("satura\n");
        duty3 = dutymax;
      } else if (duty3 < dutymin){
        duty3 = dutymin;
      } 

    if (iroll > MAXIROLL){
      iroll = MAXIROLL;
    }

    if (iroll < -MAXIROLL){
      iroll =  - MAXIROLL;
    }

    if (ipitch > MAXIPITCH){
      ipitch = MAXIPITCH;
    }

    if (ipitch < -MAXIPITCH){
      ipitch = - MAXIPITCH;
    }

    if (!gpio_pin_get_dt(&button) && !flagcair) { 
        pwm_set_dt(pwms[0], period, duty0);
        pwm_set_dt(pwms[1], period, duty1);
        pwm_set_dt(pwms[2], period, duty2);
        pwm_set_dt(pwms[3], period, duty3);
        printk("%d: P = %f R = %f \n",flagbyte , pitch + offp, roll + offr);


    } else { // sem botão, deligar PWM
        pwm_set_dt(pwms[0], period, duty);
        pwm_set_dt(pwms[1], period, duty);
        pwm_set_dt(pwms[2], period, duty);
        pwm_set_dt(pwms[3], period, duty);
        yaw = 0;
        ipitch = 0;
        iroll = 0;
        printk ("P = %f R = %f delta = %f \n", pitch, roll, dpitch);
        //printk ("dutys: 1 = %d 2 = %d 3 = %d 4 = %d  certo = %d \n", (duty0 * 100)/ PERIOD , (duty1 * 100)/ PERIOD, (duty2 * 100)/ PERIOD, (duty3 * 100)/ PERIOD , (duty * 100)/ PERIOD);
        //printk ("offset: P = %f R = %f\n", offp, offr);
    }
  }

  return 0;
}