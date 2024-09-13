#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>
int cn = 1;
/*
RISING (рост) - срабатывает при изменении сигнала с LOW на HIGH
FALLING (падение) - срабатывает при изменении сигнала с HIGH на LOW
CHANGE (изменение) - срабатывает при изменении сигнала (с LOW на HIGH и наоборот)
LOW (низкий) - срабатывает постоянно при сигнале LOW
(не поддерживается на ESP8266)
*/

/* сдвиг после считывания оборота вала со шпулей   to es sdvig*/

struct Data {
  int DiametroMini = 2.5;
  int DiametroMaxi = 4.5;
  int largo = 15;
  float fvalue = 4.5;
};
// глобальный экземпляр для личного использования
Data data;
long timpoPas = 0;      // временная
int ps = 0;             // временная
long largos = 0;        // временная
long x = 0;             // нажатая цифра
long n = 0;             // число на вводе
long tiempoNumber = 0;  // наборная из клавиатуры

const int pinProgreso = 10;  // пин сдвига
int progreso = 145;          ///сдвиг на 4,5мм
//Логический флаг для сдвига
bool flagProgreso = 0;

/* сдвиг после считывания оборота вала со шпулей*/
/*
сдвиг после считывания оборота вала со шпулей

Тлп = Sшд*Fшд/Pр*Nшк

где:

Тлп - точность линейного перемещения, шаг/мм
Sшд — количество шагов на оборот для двигателя (в наших примерах 200)
Fшд — микрошаг (1, 2, 4, 8 и т. д.)
Pр — шаг ремня (например, 2 мм)
Nшк — количество зубьев на шкиве, на валу двигателя.
Попробуем посчитать для примера с такими параметрами, двигатель 200 шагов на оборот, с 2-кратным микрошагом, 2-миллиметровыми ремнями GT2 и шкивом с 20 зубцами даст нам 10 шагов на мм.

200*2/2*20=10 шагов/мм.
*/

const long stepMotoApilador = 9;  // Steppin мотор укладчика

const int dereccion = 7;  // DirPin направление мотор укладчика

const int EnablePin = 8;  //EnablePin   остановка мотора
#define frequency 2250    //Время между импульсами в мксек. 1000 мксек = 1 мсек = 0.001 сек. Частота следования импульсов 1/0.001 = 1 кГц,
//Не рекомендуется устанавливать время меньше 100 мсек, т.к. частота будет 10 кГц

//Логический флаг для рабочего режима
bool flagTrabajadora = 0;

const int disposicion = A1;  // конечное положение
const int izcuierda = A2;    // левая точка
const int derecha = A3;      // правая точка

int testDiod = 4;  //////////////////////подключаем светодиод

LiquidCrystal_I2C lcd(0x27, 16, 2);  // экземпляр жк экрана

bool puntoIzcuierdaFlag = false;  // флаг левого положения
bool puntoDerechaFlag = false;    // флаг правого положения
uint32_t btnTimer = 0;            // таймер кнопок

volatile int counter = 0;       // переменная-счётчик
volatile bool intFlag = false;  // флаг
int houser = 0;                 // сделать память
bool status;
const int PinLamp = 5;       // заменить на цикл укладчика
const int recalculo = A0;    // кнопка пересчета длинны
bool recalculoFlag = false;  // флаг
int moto = 6;                // motor rele

void setup() {

  /* EEPROM.get(0, data); // прочитать из адреса 0
                            //  меняем
       data.largo = 12;
       data.fvalue = 3.14;
       EEPROM.put(0, data); // поместить в EEPROM по адресу 0
   */
  Serial.begin(9600);  // открыли порт для связи
                       // Keyboard.begin(); // открыли клавиатуры
  lcd.init();

  lcd.backlight();

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print(String("BOBINADORA"));
  delay(1000);
  lcd.setCursor(0, 1);  // ввод
  lcd.print("www.katalina.ru");
  delay(2000);


  pinMode(pinProgreso, INPUT_PULLUP);  // кнопка оборота шпинделя
  pinMode(disposicion, INPUT_PULLUP);  // кнопка дома
  pinMode(EnablePin, OUTPUT);          // stop  уладчик
  digitalWrite(EnablePin, 1);          // состояние выключен мотор чтобы не грелся
  pinMode(dereccion, OUTPUT);          // направление уладчик
  digitalWrite(dereccion, 0);          // состояние
  pinMode(stepMotoApilador, OUTPUT);   // уладчик
  digitalWrite(stepMotoApilador, 0);   // состояние


  pinMode(izcuierda, INPUT_PULLUP);  // точка лево
  pinMode(derecha, INPUT_PULLUP);    // точка право

  pinMode(moto, OUTPUT);  // мотор намотки реле
  digitalWrite(moto, 1);  // выход надо проверить + или -

  pinMode(recalculo, INPUT_PULLUP);  // пин кнопки входа в память
  // подключили кнопку на D2 и GND

  pinMode(PinLamp, OUTPUT);
  digitalWrite(moto, 1);
  pinMode(testDiod, OUTPUT);  /////////////////////////////
  pinMode(2, INPUT_PULLUP);

  //  pinMode(2, INPUT);
  // FALLING - при нажатии на кнопку будет сигнал 0, его и ловим
  // attachInterrupt(0, btnIsr, FALLING);
  digitalWrite(PinLamp, 1);  //  сигнал 0 на реле
  // attachInterrupt(0, buttonTick, RISING); //+
  // attachInterrupt(0, buttonTick, FALLING);//+
  attachInterrupt(0, buttonTick, CHANGE);  //++
                                           // attachInterrupt(0, buttonTick, LOW);
  lcd.clear();
}
//*****************************************************************

void Aplicador()  // укладчик/////////////************************
{
}
///--------------------------
void MiEeprom(long a) {
  EEPROM.get(0, data);  // прочитать из адреса 0
                        //  меняем

  data.largo = a;
  data.fvalue = 3.14;   // запасные данныые
  EEPROM.put(0, data);  // поместить в EEPROM по адресу 0
}

void Recalculo() {
  bool btnState = !digitalRead(recalculo);
  if (btnState && !recalculoFlag) {  // обработчик нажатия
    recalculoFlag = true;
    Serial.println("press");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(String("Trabajo ") + data.largo);
    delay(2000);
    Serial.println(String("Trabajo "));
    // обнуление счетчика
    counter = 0;
    digitalWrite(moto, 0);
    // !!!!!!11 включился мотор укладчика надо заменить на flag
    flagTrabajadora = 1;

    lcd.clear();
  }
  if (!btnState && recalculoFlag) {  // обработчик отпускания
    recalculoFlag = false;
    ///////выйти в перезагрузку
  }
  // вызываем клавиатуру
  // Serial.println(String("enciende el teclado"));  // выводим
}
void buttonTick()  // сработка от прерывания
{
  intFlag = true;  // подняли флаг прерывания

  if (intFlag && counter < data.largo) {
    intFlag = false;  // сбрасываем
    // совершаем какие-то действия
    counter++;  // + нажатие
    Serial.println(counter);
    digitalWrite(PinLamp, 0);  //  сигнал 0 на реле

    digitalWrite(testDiod, 1);  //  сигнал diod
  }

  else if (counter >= data.largo) {
    digitalWrite(PinLamp, 1);   //  сигнал 0 на реле
    digitalWrite(testDiod, 0);  //  сигнал diod

    digitalWrite(moto, 1);  // отключаем реле мотора и останавливае укладчик
    //digitalWrite(stepMotoApilador, 0); // стоп мотор
    flagTrabajadora = 0;
    digitalWrite(dereccion, 0);  // сбрасываем направление

    Serial.println("Fin");


    counter = 0;
  }
}
//***********
void Derecha() {

  // читаем инвертированное значение для удобства
  bool btnState1 = !digitalRead(derecha);
  if (btnState1 && !puntoDerechaFlag && millis() - btnTimer > 100) {
    puntoDerechaFlag = true;
    btnTimer = millis();
    Serial.println("press puntoDerechaFlag");

    digitalWrite(EnablePin, 1);  //  выключаем мотор
    delay(50);
    digitalWrite(dereccion, 1);  // мотор меняем направление
    digitalWrite(EnablePin, 0);  //  включаем мотор
  }
  if (!btnState1 && puntoDerechaFlag && millis() - btnTimer > 100) {
    puntoDerechaFlag = false;
    btnTimer = millis();
    // Serial.println("release");
  }
  ///
  /****************/  ///
  bool btnState2 = !digitalRead(izcuierda);
  if (btnState2 && !puntoIzcuierdaFlag && millis() - btnTimer > 100) {
    puntoIzcuierdaFlag = true;
    btnTimer = millis();
    Serial.println("press puntoIzcuierdaFlag");

    digitalWrite(EnablePin, 1);  //  выключаем мотор
    delay(50);
    digitalWrite(dereccion, 0);  // мотор меняем направление
    digitalWrite(EnablePin, 0);  //  включаем мотор
  }
  if (!btnState2 && puntoIzcuierdaFlag && millis() - btnTimer > 100) {
    puntoIzcuierdaFlag = false;
    btnTimer = millis();
    // Serial.println("release");
  }
}

///// после сработки датчика шпули запустить работуъ
void Trabajadora() {


  if (flagTrabajadora && flagProgreso) {

    for (int n = 0; n < progreso; n++)  ////// ghjmmm
    {
      Serial.println("to es sdvig");
      digitalWrite(stepMotoApilador, HIGH);  // мотор
      delayMicroseconds(frequency);
      digitalWrite(stepMotoApilador, LOW);  //стоп мотор
    }
    flagProgreso = false;
    //flagTrabajadora =0;
  } else {
    digitalWrite(stepMotoApilador, LOW);  //стоп мотор
  }
}


void Disposicion() {
  bool btnStateDisposicion = !digitalRead(disposicion);
  if (btnStateDisposicion && digitalRead(izcuierda) && millis() - btnTimer > 500) {
    //puntoDerechaFlag = true;
    btnTimer = millis();
    Serial.println("press Disposicion");

    digitalWrite(EnablePin, 1);  //  выключаем мотор
                                 //  delay(50);
    digitalWrite(dereccion, 1);  // мотор меняем направление
    digitalWrite(EnablePin, 0);  //  включаем мотор
    // переделать условия под концевик
    while (!digitalRead(disposicion) && digitalRead(izcuierda)) {
      digitalWrite(stepMotoApilador, HIGH);  // мотор
      delayMicroseconds(frequency);
      digitalWrite(stepMotoApilador, LOW);  //стоп мотор
    }
  }
  if (!btnStateDisposicion && puntoIzcuierdaFlag && millis() - btnTimer > 100) {
    puntoDerechaFlag = false;
    btnTimer = millis();
    // Serial.println("release");
  }
}

void Progreso() {
  /*
  progreso
  flagProgreso
  */

  bool btnStateProgreso = !digitalRead(pinProgreso);
  if (btnStateProgreso && !flagProgreso && millis() && flagTrabajadora) {
    flagProgreso = true;
    btnTimer = millis();
    Serial.println(cn);
    ++cn;
    Serial.println("press flagProgreso = true");
  }
}
//*************************************************************
void loop() {
  {
    // lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print(String(counter));
  }

  if (counter > 0) {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print(String(counter));
    // delay(100);
  }

  ////--------------------*******
  Derecha();
  Recalculo();
  Trabajadora();
  Disposicion();
  Progreso();
}
