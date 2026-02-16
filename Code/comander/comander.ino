#include <SPI.h>
#include <RF24.h>

// --- Настройки радиомодуля ---
RF24 radio(9, 10); // CE, CSN
const byte addressTX[6] = "PIPE1"; // Адрес для отправки
const byte addressRX[6] = "PIPE2"; // Адрес для приема отчетов

// --- Пины ---
const int buttonPin = 2;

// --- Состояние ---
int lastButtonState = HIGH;
byte commandCounter = 1; // Счётчик для команд (1, 2, 3, 4)

// Структура для приема отчета (остается без изменений)
struct FeedbackPacket {
  byte servo1_angle;
  byte servo2_angle;
};

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);

  if (!radio.begin()) {
    Serial.println("Ошибка: Модуль NRF24L01 не отвечает!");
    while (1) {}
  }
  radio.setPALevel(RF24_PA_MIN); // Минимальная мощность
  radio.setDataRate(RF24_250KBPS); // Самая низкая скорость (самая высокая дальнобойность и стабильность)
  // Настройка двусторонней связи
  radio.openWritingPipe(addressTX);
  radio.openReadingPipe(1, addressRX);
  radio.setPALevel(RF24_PA_MIN);

  radio.startListening(); // Начинаем слушать эфир
  Serial.println("Передатчик готов. Нажмите кнопку для отправки команды.");
}

void loop() {
  // --- ЧАСТЬ 1: ПРИЕМ ОТЧЕТОВ ОТ ПРИЕМНИКА ---
  if (radio.available()) {
    FeedbackPacket feedback;
    radio.read(&feedback, sizeof(FeedbackPacket));
    Serial.print("ОТЧЕТ: Серво1 = ");
    Serial.print(int(feedback.servo1_angle)-90);
    Serial.print("°, Серво2 = ");
    Serial.print(int(feedback.servo2_angle)-90);
    Serial.println("°");
  }

  // --- ЧАСТЬ 2: ОТПРАВКА КОМАНДЫ ПО НАЖАТИЮ КНОПКИ ---
  int currentButtonState = digitalRead(buttonPin);

  // Проверяем, что кнопка была нажата (переход из HIGH в LOW)
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    delay(50); // Небольшая задержка для защиты от дребезга контактов

    radio.stopListening(); // Прекращаем слушать для отправки данных
    radio.write(&commandCounter, sizeof(commandCounter)); // Отправляем текущий номер команды
    radio.startListening(); // Сразу же возобновляем прослушивание

    Serial.print("-> Команда '");
    Serial.print(commandCounter);
    Serial.println("' отправлена!");

    // // Увеличиваем счётчик для следующего нажатия
    // commandCounter++;
    // if (commandCounter > 4) {
    //   commandCounter = 1; // Сбрасываем на 1, если достигли 5
    // }
  }

  lastButtonState = currentButtonState; // Обновляем состояние кнопки
}