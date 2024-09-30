/*
  Максимально быстрый универсальный код для обработки энкодера
  Работает на перывании (используется одно)
  Тут код построен на digitalRead, что делает его универсальным для всех плат Arduino
  const int TickA = 2;  // пин счетчик А
const int TickB = 3;  // пин счетчик B
*/
#define TickA 2    // пин энкодера TickA
#define TickB 5    // пин энкодера
#define ENC_TYPE 1 // тип энкодера, 0 или 1
volatile int encCounter;
volatile boolean state0, lastState, turnFlag;
void setup()
{
  Serial.begin(9600);
  attachInterrupt(0, int0, CHANGE);
}

void int0()
{
  state0 = digitalRead(TickA);
  if (state0 != lastState)
  {
#if (ENC_TYPE == 1)
    turnFlag = !turnFlag;
    if (turnFlag)
      encCounter += (digitalRead(TickB) != lastState) ? -1 : 1;
#else
    encCounter += (digitalRead(TickB) != lastState) ? -1 : 1;
#endif
    lastState = state0;
  }
}

void loop()
{
  Serial.println(encCounter);
  delay(500);
}