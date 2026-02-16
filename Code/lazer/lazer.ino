#include <Servo.h>
#include <SPI.h>
#include <RF24.h>

// --- Объекты ---
Servo servo1;
Servo servo2;
RF24 radio(8, 10);

// Адреса меняются местами!
const byte addressRX[6] = "PIPE1"; // Слушаем команды из "PIPE1"
const byte addressTX[6] = "PIPE2"; // Отправляем отчеты в "PIPE2"

// --- Пины ---
const int Servo1_pin = 6;
const int Servo2_pin = 5;
const int Lazer_Pin = 7;

// --- Задержка ---

const int del=3000;

// Структура для отправки отчета
struct FeedbackPacket {
  byte servo1_angle;
  byte servo2_angle;
};

// --- Вспомогательная функция для отправки отчета ---
void sendFeedback() {
  radio.stopListening(); // Прекращаем слушать
  FeedbackPacket feedback;
  feedback.servo1_angle = servo1.read(); // Читаем текущий угол
  feedback.servo2_angle = servo2.read(); // Читаем текущий угол
  radio.write(&feedback, sizeof(FeedbackPacket)); // Отправляем отчет
  radio.startListening(); // Сразу же начинаем слушать снова
}

void setup() {
  Serial.begin(9600);
  servo1.attach(Servo1_pin);
  servo2.attach(Servo2_pin);
  pinMode(Lazer_Pin, OUTPUT);
  digitalWrite(Lazer_Pin, HIGH);
  servo1.write(90);
  servo2.write(90);

  if (!radio.begin()) {
    Serial.println("Ошибка: Модуль NRF24L01 не отвечает!");
    while (1) {}
  }
  radio.setPALevel(RF24_PA_MIN); // Минимальная мощность
  radio.setDataRate(RF24_250KBPS); // Самая низкая скорость (самая высокая дальнобойность и стабильность)
  // Настраиваем двустороннюю связь
  radio.openWritingPipe(addressTX);
  radio.openReadingPipe(1, addressRX);
  radio.setPALevel(RF24_PA_MIN);
  
  radio.startListening();
  Serial.println("Приемник готов. Ожидание команд...");
}

void loop() {
  if (radio.available()) {
    byte commandID; // Принимаем одну цифру
    radio.read(&commandID, sizeof(commandID));
    Serial.print("<- Получена команда: ");
    Serial.println(commandID);

    // Выбираем, какую последовательность запустить
    switch (commandID) {
      case 1: allaxis(); break; 
    }
    Serial.println("Последовательность завершена. Возврат в режим ожидания.");
  }
}
void allaxis(){
  turnX();
  turnY();
  turnXY();
  turn_negXY();
}
// --- Последовательности движений с отправкой отчетов ---
void turnX() {
  digitalWrite(Lazer_Pin, HIGH);
  for(int i = 100; i <= 130; i += 10) { servo1.write(i); delay(del); sendFeedback(); }
  for(int i = 120; i >= 50; i -= 10) { servo1.write(i); delay(del); sendFeedback(); }
  for(int i = 60; i <= 90; i += 10) { servo1.write(i); delay(del); sendFeedback(); }
  digitalWrite(Lazer_Pin, LOW);
}

void turnY() {
  digitalWrite(Lazer_Pin, HIGH);
  for(int i = 100; i <= 130; i += 10) { servo2.write(i); delay(del); sendFeedback(); }
  for(int i = 120; i >= 50; i -= 10) { servo2.write(i); delay(del); sendFeedback(); }
  for(int i = 60; i <= 90; i += 10) { servo2.write(i); delay(del); sendFeedback(); }
  digitalWrite(Lazer_Pin, LOW);
}

void turnXY() {
  digitalWrite(Lazer_Pin, HIGH);
  for(int i = 100; i <= 130; i += 10) { servo1.write(i); servo2.write(i); delay(del); sendFeedback(); }
  for(int i = 120; i >= 50; i -= 10) { servo1.write(i); servo2.write(i); delay(del); sendFeedback(); }
  for(int i = 60; i <= 90; i += 10) { servo1.write(i); servo2.write(i); delay(del); sendFeedback(); }
  digitalWrite(Lazer_Pin, LOW);
}

void turn_negXY() {
  digitalWrite(Lazer_Pin, HIGH);
  for(int i = 10; i <= 40; i+=10) { servo1.write(90 - i); servo2.write(90 + i); delay(del); sendFeedback(); }
  for(int i = 30; i >= -40; i-=10) { servo1.write(90 - i); servo2.write(90 + i); delay(del); sendFeedback(); }
  for(int i = -30; i <= 0; i+=10) { servo1.write(90 - i); servo2.write(90 + i); delay(del); sendFeedback(); }
  digitalWrite(Lazer_Pin, LOW);
}