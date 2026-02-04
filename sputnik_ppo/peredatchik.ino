#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN

// Используем ДВА адреса: один для отправки, другой для приема
const byte addressTX[6] = "PIPE1"; // Адрес, на который отправляем команды
const byte addressRX[6] = "PIPE2"; // Адрес, с которого слушаем отчеты

// --- Пины ---
const int potPin = A0;
const int buttonPin = 2;
const int ledPin1 = 3;
const int ledPin2 = 4;
const int ledPin3 = 5;
const int ledPin4 = 6;

// --- Состояние ---
int lastButtonState = HIGH; // Убрали const, т.к. переменная меняется

// Структура для приема отчета об углах от приемника
struct FeedbackPacket {
  byte servo1_angle;
  byte servo2_angle;
};

void setup() {
  Serial.begin(9600); // Эта строка должна быть в самом начале!

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);

  if (!radio.begin()) {
    Serial.println("Ошибка: Модуль NRF24L01 не отвечает!");
    while (1) {}
  }

  // Настраиваем двустороннюю связь
  radio.openWritingPipe(addressTX);     // Говорим в "PIPE1"
  radio.openReadingPipe(1, addressRX);  // Слушаем из "PIPE2"
  radio.setPALevel(RF24_PA_MIN);

  radio.startListening(); // Начинаем слушать эфир по умолчанию
  Serial.println("Передатчик готов. Ожидание отчетов...");
}

void loop() {
  // --- ЧАСТЬ 1: ПРИЕМ ОТЧЕТОВ ОТ ПРИЕМНИКА ---
  if (radio.available()) {
    FeedbackPacket feedback;
    radio.read(&feedback, sizeof(FeedbackPacket));
    Serial.print("ОТЧЕТ: Серво1 = ");
    Serial.print(feedback.servo1_angle);
    Serial.print("°, Серво2 = ");
    Serial.print(feedback.servo2_angle);
    Serial.println("°");
  }

  // --- ЧАСТЬ 2: УПРАВЛЕНИЕ СВЕТОДИОДАМИ И ОТПРАВКА КОМАНД ---
  int potValue = analogRead(potPin);
  
  // Управляем светодиодами
  if (potValue < 256) {
    digitalWrite(ledPin1, HIGH); digitalWrite(ledPin2, LOW); digitalWrite(ledPin3, LOW); digitalWrite(ledPin4, LOW);
  } else if (potValue < 512) {
    digitalWrite(ledPin1, LOW); digitalWrite(ledPin2, HIGH); digitalWrite(ledPin3, LOW); digitalWrite(ledPin4, LOW);
  } else if (potValue < 768) {
    digitalWrite(ledPin1, LOW); digitalWrite(ledPin2, LOW); digitalWrite(ledPin3, HIGH); digitalWrite(ledPin4, LOW);
  } else {
    digitalWrite(ledPin1, LOW); digitalWrite(ledPin2, LOW); digitalWrite(ledPin3, LOW); digitalWrite(ledPin4, HIGH);
  }

  // Проверяем нажатие кнопки
  int currentButtonState = digitalRead(buttonPin);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    byte commandToSend = 0; // 0 - нет команды

    // Определяем, какая команда соответствует горящему светодиоду
    if (digitalRead(ledPin1) == HIGH) commandToSend = 1;
    else if (digitalRead(ledPin2) == HIGH) commandToSend = 2;
    else if (digitalRead(ledPin3) == HIGH) commandToSend = 3;
    else if (digitalRead(ledPin4) == HIGH) commandToSend = 4;

    if (commandToSend > 0) {
      radio.stopListening(); // ПРЕКРАЩАЕМ слушать
      radio.write(&commandToSend, sizeof(commandToSend)); // ОТПРАВЛЯЕМ команду
      radio.startListening(); // СРАЗУ ЖЕ НАЧИНАЕМ слушать снова
      Serial.print("-> Команда '");
      Serial.print(commandToSend);
      Serial.println("' отправлена!");
    }
  }
  lastButtonState = currentButtonState; // Обновляем состояние кнопки
}