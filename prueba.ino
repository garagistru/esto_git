#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>

/*
RISING (рост) - срабатывает при изменении сигнала с LOW на HIGH
FALLING (падение) - срабатывает при изменении сигнала с HIGH на LOW
CHANGE (изменение) - срабатывает при изменении сигнала (с LOW на HIGH и наоборот)
LOW (низкий) - срабатывает постоянно при сигнале LOW
(не поддерживается на ESP8266)
*/

struct Data
{
    int DiametroMini = 2.5;
    int DiametroMaxi = 4.5;
    int largo = 15;
    float fvalue = 4.5;
};
// глобальный экземпляр для личного использования
Data data;
long timpoPas = 0;     // временная
int ps = 0;            // временная
long largos = 0;       // временная
long x = 0;            // нажатая цифра
long n = 0;            // число на вводе
long tiempoNumber = 0; // наборная из клавиатуры

const int stepMotoApilador = 9; // мотор укладчика

const int dereccion = A1; // направление

// const int delKeypad2= xxxxxx;  //кнопка вызова клавиатуры
bool flagKeypad2 = false; // флаг кнопки клавы

const int izcuierda = A2; // левая точка
const int derecha = A3;   // правая точка

int testDiod = 4; //////////////////////
// клавиатура
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

byte rowPin[ROWS] = {13, 12, 11, 10};
byte colPin[COLS] = {9, 8, 7};

// Keypad kpd = Keypad(makeKeymap(keys), rowPin, colPin, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2); // экземпляр жк экрана

bool puntoIzcuierdaFlag = false; // флаг левого положения
bool puntoDerechaFlag = false;   // флаг правого положения
uint32_t btnTimer = 0;           // таймер кнопок

volatile int counter = 0;      // переменная-счётчик
volatile bool intFlag = false; // флаг
int houser = 0;                // сделать память
bool status;
const int PinLamp = 5;      // заменить на цикл укладчика
const int recalculo = A0;   // кнопка пересчета длинны
bool recalculoFlag = false; // флаг
int moto = 6;               // motor rele

void setup()
{

    /* EEPROM.get(0, data); // прочитать из адреса 0
                            //  меняем

       data.largo = 12;
       data.fvalue = 3.14;
       EEPROM.put(0, data); // поместить в EEPROM по адресу 0
   */
    Serial.begin(9600); // открыли порт для связи
                        // Keyboard.begin(); // открыли клавиатуры
    lcd.init();

    lcd.backlight();

    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print(String("BOBINADORA"));
    delay(1000);
    lcd.setCursor(0, 1); // ввод
    lcd.print("www.katalina.ru");
    delay(2000);

    // pinMode(delKeypad2, INPUT_PULLUP);  //кнопка вызова клавиатуры
    pinMode(dereccion, OUTPUT); // направление уладчик
    digitalWrite(dereccion, 0); // состояние

    pinMode(izcuierda, INPUT_PULLUP); // точка лево
    pinMode(derecha, INPUT_PULLUP);   // точка право

    pinMode(stepMotoApilador, OUTPUT); // уладчик
    digitalWrite(stepMotoApilador, 0); // состояние

    pinMode(moto, OUTPUT); // мотор намотки реле
    digitalWrite(moto, 1); // выход надо проверить + или -

    pinMode(recalculo, INPUT_PULLUP); // пин кнопки входа в память
    // подключили кнопку на D2 и GND

    pinMode(PinLamp, OUTPUT);
    digitalWrite(moto, 1);
    pinMode(testDiod, OUTPUT); /////////////////////////////
    pinMode(2, INPUT_PULLUP);

    //  pinMode(2, INPUT);
    // FALLING - при нажатии на кнопку будет сигнал 0, его и ловим
    // attachInterrupt(0, btnIsr, FALLING);
    digitalWrite(PinLamp, 1); //  сигнал 0 на реле
    // attachInterrupt(0, buttonTick, RISING); //+
    // attachInterrupt(0, buttonTick, FALLING);//+
    attachInterrupt(0, buttonTick, CHANGE); //++
                                            // attachInterrupt(0, buttonTick, LOW);
    lcd.clear();
}
//*****************************************************************
///----------------------------
///-------------------------------
void Aplicador() // укладчик/////////////************************
{
}
///--------------------------
void MiEeprom(long a)
{
    EEPROM.get(0, data); // прочитать из адреса 0
                         //  меняем

    data.largo = a;
    data.fvalue = 3.14;  // запасные данныые
    EEPROM.put(0, data); // поместить в EEPROM по адресу 0
}
//---------------------------------
int DetectButtons();
//------------------------
void Hablo(String a);
void Hablo_2(String a);
/////////----------------------------------------------
/////////////-----------------------------------------------
//-----------------------------------------------//
void Recalculo()
{
    bool btnState = !digitalRead(recalculo);
    if (btnState && !recalculoFlag)
    { // обработчик нажатия
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
        // !!!!!!11 включился мотор укладчика надо заменить на функцию  Derecha
        for (int n = 0; n < 10; n++)
        {
            digitalWrite(stepMotoApilador, 1); // мотор
            delay(100);
            //  digitalWrite(motoApilador, 0);//стоп мотор
            delay(100);
        }
        lcd.clear();
    }
    if (!btnState && recalculoFlag)
    { // обработчик отпускания
        recalculoFlag = false;
        ///////выйти в перезагрузку
    }
    // вызываем клавиатуру
    // Serial.println(String("enciende el teclado"));  // выводим
}
void buttonTick() // сработка от прерывания
{
    intFlag = true; // подняли флаг прерывания

    if (intFlag && counter < data.largo)
    {
        intFlag = false; // сбрасываем
        // совершаем какие-то действия
        counter++; // + нажатие
        Serial.println(counter);
        digitalWrite(PinLamp, 0); //  сигнал 0 на реле

        digitalWrite(testDiod, 1); //  сигнал diod
    }

    else if (counter >= data.largo)
    {
        digitalWrite(PinLamp, 1);  //  сигнал 0 на реле
        digitalWrite(testDiod, 0); //  сигнал diod

        digitalWrite(moto, 1);         // отключаем реле мотора и останавливае укладчик
        digitalWrite(stepMotoApilador, 0); // стоп мотор
        digitalWrite(dereccion, 0);    // сбрасываем направление

        Serial.println("Fin");

        counter = 0;
    }
}
//***********
void btnIsr()
{
    counter++; // + нажатие
    status = 1;

    Serial.println(counter); // выводим

    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print(String("BOBINADORA"));
    delay(1000);
    lcd.setCursor(0, 1); // ввод
    lcd.print("www.katalina.ru");
    delay(2000);
}
//**************
void Fu()
{
    Serial.println("FU"); // выводим
    Fu2();
}

void Fu2()
{
    Serial.println("FU_2"); // выводим
}
//************************************
//*************************************************************
void Derecha()
{

    // читаем инвертированное значение для удобства
    bool btnState1 = !digitalRead(derecha);
    if (btnState1 && !puntoDerechaFlag && millis() - btnTimer > 100)
    {
        puntoDerechaFlag = true;
        btnTimer = millis();
        Serial.println("press puntoDerechaFlag");

        // включить motoApilador
        digitalWrite(stepMotoApilador, 0); // мотор              функция степ мотор
        delay(50);

        // включить направление
        digitalWrite(dereccion, 1);    // мотор
        digitalWrite(stepMotoApilador, 1); // мотор функция степ мотор
    }
    if (!btnState1 && puntoDerechaFlag && millis() - btnTimer > 100)
    {
        puntoDerechaFlag = false;
        btnTimer = millis();
        // Serial.println("release");
    }
    ///
    /****************/ ///
    bool btnState2 = !digitalRead(izcuierda);
    if (btnState2 && !puntoIzcuierdaFlag && millis() - btnTimer > 100)
    {
        puntoIzcuierdaFlag = true;
        btnTimer = millis();
        Serial.println("press puntoIzcuierdaFlag");

        digitalWrite(stepMotoApilador, 0); // мотор                          функция степ мотор
        delay(50);
        // включить направление
        digitalWrite(dereccion, 0);    // направление
        digitalWrite(stepMotoApilador, 1); // мотор /////////////////функция степ мотор
    }
    if (!btnState2 && puntoIzcuierdaFlag && millis() - btnTimer > 100)
    {
        puntoIzcuierdaFlag = false;
        btnTimer = millis();
        // Serial.println("release");
    }
}
//*************************************************************
void loop()
{
    {
        // lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print(String(counter));
    }

    if (counter > 0)
    {
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print(String(counter));
        delay(100);
    }
    ////--------------------*******
    Derecha();
    Recalculo();
}
///--------------------------------------//
//************************************************/
void Hablo(String a)
{
    lcd.setCursor(1, 0);
    lcd.print(a);
}
//////////////---------------------------------------------
void Hablo_2(String a)
{
    lcd.setCursor(2, 1);
    lcd.print(a);
}
