#include <MPU6050_light.h>  //"MPU6050_light", rfetick
#include <Wire.h>
#include <Servo.h>

MPU6050 mpu(Wire);

Servo motor0, motor1, motor2, motor3;

const int BUTTON_PIN = 2;

#define PERIOD 2500  // in arduino we use micro

// CONSTANTES DE CONTROLE (SÓ PROPORCIONAL POR ENQUANTO) (exceto dy, que é integral)
#define kpp 0
#define kpr 0
#define kiy 0
#define base 30 * PERIOD / 100

// saturações motores
#define dutymax  8 * PERIOD / 10  // saturação do motor
#define dutymin  4 * PERIOD / 10  // minimo do motor em voo

void setup() {
  Serial.begin(115200);
  Wire.begin();
  byte status = mpu.begin();

  if (status != 0) {
    Serial.print("Device MPU6050 is not ready: ");
    Serial.println(status);
    while (1);
  }

  mpu.calcOffsets(); // gyro and accel auto-calibration

  motor0.attach(1);
  motor1.attach(2);
  motor2.attach(3);
  motor3.attach(4);

  pinMode(BUTTON_PIN, INPUT);

  int duty = 10 * PERIOD / 4; // duty de 40% para conexão com escs
  motor0.writeMicroseconds(map(duty, 0, PERIOD, 1000, 2000));
  motor1.writeMicroseconds(map(duty, 0, PERIOD, 1000, 2000));
  motor2.writeMicroseconds(map(duty, 0, PERIOD, 1000, 2000));
  motor3.writeMicroseconds(map(duty, 0, PERIOD, 1000, 2000));

  Serial.println("Connecting ESCs, wait 5 seconds for setup");
  delay(5000);

  if (digitalRead(BUTTON_PIN) == HIGH) {
    while (digitalRead(BUTTON_PIN) == HIGH) {
      Serial.println("Desligue o botão e ligue de novo para iniciar");
      delay(500);
    }
  }
}

void loop() {

  mpu.update();

  float pitch = -mpu.getAccX();
  float roll  = -mpu.getAccY();
  float dyaw  =  mpu.getGyroZ();

  static float yaw = 0;
  yaw = yaw + dyaw;

  // se quiser printar accell
  Serial.print("Accel: X="); 
  Serial.print(mpu.getAccX());
  Serial.print(", Y=");       
  Serial.print(mpu.getAccY());
  Serial.print(", Z=");       
  Serial.println(mpu.getAccZ());
  
  // se quiser printar gyro
  Serial.print("Gyro:  X=");
  Serial.print(mpu.getGyroX());
  Serial.print(", Y=");
  Serial.print(mpu.getGyroY());
  Serial.print(", Z=");
  Serial.println(mpu.getGyroZ());

  //CONTAS em MICROSEGUNDOS (2500 é 100%)
  int duty0 = base - (kpp * pitch) + (kpr * roll) - (kiy * yaw);
  int duty1 = base + (kpp * pitch) - (kpr * roll) - (kiy * yaw);
  int duty2 = base - (kpp * pitch) - (kpr * roll) + (kiy * yaw);
  int duty3 = base + (kpp * pitch) + (kpr * roll) + (kiy * yaw); // was overwriting duty0

  if (digitalRead(BUTTON_PIN) == HIGH) { // se botão ligado, ativar motores
    //MOTOR1:
    if (duty0 > dutymax){
      motor0.writeMicroseconds(map(dutymax, 0, PERIOD, 1000, 2000));
    } else if (duty0 < dutymin){
      motor0.writeMicroseconds(map(dutymin, 0, PERIOD, 1000, 2000));
    } else {
      motor0.writeMicroseconds(map(duty0, 0, PERIOD, 1000, 2000));
    }

    //MOTOR2:
    if (duty1 > dutymax){
      motor1.writeMicroseconds(map(dutymax, 0, PERIOD, 1000, 2000));
    } else if (duty1 < dutymin){
      motor1.writeMicroseconds(map(dutymin, 0, PERIOD, 1000, 2000));
    } else {
      motor1.writeMicroseconds(map(duty1, 0, PERIOD, 1000, 2000));
    }

    //MOTOR3:
    if (duty2 > dutymax){
      motor2.writeMicroseconds(map(dutymax, 0, PERIOD, 1000, 2000));
    } else if (duty2 < dutymin){
      motor2.writeMicroseconds(map(dutymin, 0, PERIOD, 1000, 2000));
    } else {
      motor2.writeMicroseconds(map(duty2, 0, PERIOD, 1000, 2000));
    }

    //MOTOR4:
    if (duty3 > dutymax){
      motor3.writeMicroseconds(map(dutymax, 0, PERIOD, 1000, 2000));
    } else if (duty3 < dutymin){
      motor3.writeMicroseconds(map(dutymin, 0, PERIOD, 1000, 2000));
    } else {
      motor3.writeMicroseconds(map(duty3, 0, PERIOD, 1000, 2000));
    }

  } else { // sem botão, desligar PWM
    int duty = 10 * PERIOD / 4;
    motor0.writeMicroseconds(map(duty, 0, PERIOD, 1000, 2000));
    motor1.writeMicroseconds(map(duty, 0, PERIOD, 1000, 2000));
    motor2.writeMicroseconds(map(duty, 0, PERIOD, 1000, 2000));
    motor3.writeMicroseconds(map(duty, 0, PERIOD, 1000, 2000));
    yaw = 0;
    Serial.print("coords: P = "); Serial.print(pitch);
    Serial.print(" R = ");        Serial.print(roll);
    Serial.print(" dY = ");       Serial.println(dyaw);
    Serial.print("dutys: 1 = "); Serial.print((duty0 * 100) / PERIOD);
    Serial.print(" 2 = ");       Serial.print((duty1 * 100) / PERIOD);
    Serial.print(" 3 = ");       Serial.print((duty2 * 100) / PERIOD);
    Serial.print(" 4 = ");       Serial.println((duty3 * 100) / PERIOD);
  }

  delayMicroseconds(1);
}
