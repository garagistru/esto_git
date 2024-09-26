void buttonTick2() // сработка от прерывания для сдвига
{
    uint32_t btnTimer = 0;

    bool btnStateTick2 = !digitalRead(3);
    if (btnStateTick2 && !intFlag&&millis() -btnTimer>100)
    {
        intFlag = true;
         ++counterTick; // + нажатие
        Serial.print("moto\t");
        Serial.println(counterTick);
    }

    if (intFlag && counterTick < magneto)
    {
        //     delay(100);
        intFlag = false; // сбрасываем
       
                         // совершаем какие-то действия
    }

    if (counterTick >= magneto)
    {
        // digitalWrite(PinLamp, 1);   //  сигнал 0 на реле
        //  digitalWrite(testDiod, 0);  //  сигнал diod

        //  digitalWrite(moto, 1);  // отключаем реле мотора и останавливае укладчик
        // digitalWrite(stepMotoApilador, 0); // стоп мотор
        //  flagTrabajadora = 0;
        //  digitalWrite(dereccion, 0);  // сбрасываем направление

        Serial.println("magneto");
        /* действие для сдвига */
        flagProgreso = true;

        /* действие для сдвига */
        counterTick = 0;
    }

    if (!btnStateTick2 && intFlagTick&&millis() -btnTimer>100)
    {
        intFlag = false;
    }
}
