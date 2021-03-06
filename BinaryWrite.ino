#include <SdFat.h>
#include <SdFatConfig.h>
#include <SPI.h>

SdFat sd;
SdBaseFile ThisFile;

#define ButtonPin 16
#define SensorPin 15
#define gLED 8
#define rLED 13
#define BatteryPin 9
#define SD_CS 4
#define TestPin 11
#define MISO 22
#define MOSI 23
#define SCK 24

#define n 30
int readings[n];
byte readIndex = 0;
int total;
int SMA;

bool Switch = false;
#define CutOFF 543    //equal to 3.5V on BatteryPin
#define DelayTime 500   //delay in micros
uint32_t PastMicros = 0;

#define DebounceDelay 100  //milliseconds required for constant reading
bool ButtonState = false;
byte ButtonReading;
uint32_t PrevBounceMillis = 0;

struct DataStruct {
  int Sensor1;
  uint32_t Timing;
};
struct DataStruct InData;

void setup() {
  pinMode(ButtonPin, INPUT);
  pinMode(SensorPin, INPUT);
  pinMode(BatteryPin, INPUT);
  pinMode(rLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(TestPin, OUTPUT);
  digitalWrite(rLED, LOW);

  sd.begin(SD_CS);
  SDCARD_SPI.beginTransaction(SPISettings (48000000, MSBFIRST, SPI_MODE0));
  digitalWrite(SD_CS, HIGH);
  pinMode(SD_CS, OUTPUT);

  for (byte InitReading = 0; InitReading < n; InitReading++) {
    readings[InitReading] = 0;
  }
}

void MovingAverage() {
  total -= readings[readIndex];
  readings[readIndex] = analogRead(SensorPin);
  total += readings[readIndex];
  readIndex += 1;
  if (readIndex >= n) {
    readIndex = 0;
  }
  SMA = total / n;
}

void Debounce() {
  ButtonReading = digitalRead(ButtonPin);
  if (ButtonState == false) {
    if (ButtonReading == ButtonState) {
      PrevBounceMillis = millis();
    }
    if (millis() - PrevBounceMillis > DebounceDelay) {
      ButtonState = true;
    }
  }
  if (ButtonState == true) {
    if (ButtonReading == ButtonState) {
      PrevBounceMillis = millis();
    }
    if (millis() - PrevBounceMillis > DebounceDelay) {
      ButtonState = false;
      Open_or_Close();
    }
  }
}

void Open_or_Close () {
  if (Switch == false) {
    digitalWrite(gLED, HIGH);
    char DataFile[10];
    strcpy(DataFile, "file00.txt");
    for (byte i = 0; i < 100; i++) {
      DataFile[4] = '0' + i / 10;
      DataFile[5] = '0' + i % 10;
      if (! sd.exists(DataFile)) {
        break;
      }
    }
    ThisFile.open(DataFile, O_CREAT | O_WRONLY | O_APPEND);
    if (!sd.open(DataFile)) {
      ThisFile.close();
      Error();
      return;
    }
    Switch = true;
    delay(500);
    digitalWrite(gLED, LOW);
    return;
  }
  else {
    digitalWrite(rLED, HIGH);
    ThisFile.close();
    Switch = false;
    delay(500);
    digitalWrite(rLED, LOW);
    return;
  }
}

void loop() {
  //MovingAverage();
  Debounce();

  if (Switch == true) {
    if (micros() - PastMicros >= DelayTime) {
      PastMicros = micros();
      InData.Sensor1 = analogRead(SensorPin);
      InData.Timing = micros();
      ThisFile.write((uint8_t *)&InData, sizeof(InData));
    }
  }
}

void Error() {
  digitalWrite(rLED, HIGH);
  delayMicroseconds(100000);
  digitalWrite(gLED, HIGH);
  delayMicroseconds(100000);
  digitalWrite(rLED, LOW);
  delayMicroseconds(100000);
  digitalWrite(gLED, LOW);
  delayMicroseconds(100000);
}
