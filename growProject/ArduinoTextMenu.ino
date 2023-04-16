#include <BH1750.h>

#include <DS_raw.h>
#include <microDS18B20.h>
#include <microOneWire.h>

#include <DHTStable.h>

// ***** I2C дисплей *****
#include <LiquidCrystal_I2C.h> // https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
#include <DS1302.h>
#define cols 20
#define rows 4
LiquidCrystal_I2C lcd(0x27, cols, rows);
char *Blank;
bool autoManual = false;               // false - manual
int calibrParameters[] = {0, 0, 0, 0}; // после рестарта подгрузить из eeprom старые настройки !?

int dzien = 1;
int miesiac = 1;
int rok = 2022;
int sekundy = 0;
int minuty = 0;
int godziny = 0; // default value
// int dzien,miesiac,rok,sekundy,minuty,godziny; // resolve issue with day of week ?!
Time t(rok, miesiac, dzien, godziny, minuty, sekundy, Time::kSunday);

unsigned long myTimer1;
unsigned long myTimer2;
// relay
// #define PIN_RELAY1 13
#define PIN_RELAY2 8
#define PIN_RELAY3 7
#define PIN_RELAY4 6
// relay

//------------------------------------------------------------------------------------------------------------------------dht11
DHTStable DHT;
#define DHT11_PIN 9
//------------------------------------------------------------------------------------------------------------------------dht11

//------------------------------------------------------------------------------------------------------------------------CapacitySensor
#define ANALOGA_IN A0
//------------------------------------------------------------------------------------------------------------------------CapacitySensor

//------------------------------------------------------------------------------------------------------------------------DS18B20
MicroDS18B20<5> sensorDS18B20;
//------------------------------------------------------------------------------------------------------------------------DS18B20

//------------------------------------------------------------------------------------------------------------------------BH1750
BH1750 sensorBH1750;
//------------------------------------------------------------------------------------------------------------------------BH1750

//------------------------------------------------------------------------------------------------------------------------ds1302
const int kCePin = 10;   // RST  Chip Enable
const int kIoPin = 11;   // DAT  Input/Output
const int kSclkPin = 12; // SCLK Serial Clock

DS1302 rtc(kCePin, kIoPin, kSclkPin);

String dayAsString(const Time::Day day)
{
  switch (day)
  {
  case Time::kSunday:
    return "Niedziela";
  case Time::kMonday:
    return "Ponidzialek";
  case Time::kTuesday:
    return "Wtorek";
  case Time::kWednesday:
    return "Sroda";
  case Time::kThursday:
    return "Czwartek";
  case Time::kFriday:
    return "Piatek";
  case Time::kSaturday:
    return "Sobota";
  }
  return "(unknown day)";
}
void lastTimeOn(int *dzien, int *miesiac, int *rok, int *sekundy, int *minuty, int *godziny)
{
  Time oldTime = rtc.time();
  *rok = oldTime.yr;
  *miesiac = oldTime.mon;
  *dzien = oldTime.date;
  *godziny = oldTime.hr;
  *minuty = oldTime.min;
  *sekundy = oldTime.sec;
  // Time t(*rok, *miesiac, *dzien, *godziny, *minuty, *sekundy, Time::kSunday);
};
void printTime()
{
  // Get the current time and date from the chip.
  Time t = rtc.time();

  // Name the day of the week.
  const String day = dayAsString(t.day);

  // Format the time and date and insert into the temporary buffer.
  char buf[50];
  snprintf(buf, sizeof(buf), "%s %04d-%02d-%02d %02d:%02d:%02d",
           day.c_str(),
           t.yr, t.mon, t.date,
           t.hr, t.min, t.sec);

  // Print the formatted string to serial so we can see the time.
  Serial.println(buf);

  lcd.setCursor(0, 1);
  if (t.hr < 10)
    lcd.print("0");
  lcd.print(t.hr);
  lcd.print(":");
  if (t.min < 10)
    lcd.print("0");
  lcd.print(t.min);
  lcd.print(":");
  if (t.sec < 10)
    lcd.print("0");
  lcd.print(t.sec);

  lcd.setCursor(0, 2);
  if (t.date < 10)
    lcd.print("0");
  lcd.print(t.date);
  lcd.print("/");
  if (t.mon < 10)
    lcd.print("0");
  lcd.print(t.mon);
  lcd.print("/");
  lcd.print(t.yr);

  // lcd.setCursor(0, 3);
  // lcd.print(day);
}
//------------------------------------------------------------------------------------------------------------------------ds1302

// ********** Параметры меню **********
#define ShowScrollBar 1      // Показывать индикаторы прокрутки (0/1)
#define ScrollLongCaptions 1 // Прокручивать длинные названия (0/1)
#define ScrollDelay 800      // Задержка при прокрутке текста
#define BacklightDelay 20000 // Длительность подсветки
#define ReturnFromMenu 0     // Выходить из меню после выбора элемента(0/1)

enum eMenuKey
{
  mkNull,
  mkBack,
  mkRoot,
  mkMenu1,
  mkSettings,
  mkMenu2,
  mkEnterDzien,
  mkEnterMiesiac,
  mkEnterRok,
  mkEnterSec,
  mkEnterMin,
  mkEnterHour,
  mkMenu3,
  mkDHT11,
  mkMenu4,
  mkCapacity,
  mkMenu5,
  mkGroundTemp,
  mkMenu6,
  mkLightMeter,
  mkMenu7,
  mkRelay,
  mkMenu8,
  mkAutomatManual
};

// ********** Переменные для энкодера ***************
#define pin_CLK 2 // Энкодер пин A
#define pin_DT 4  // Энкодер пин B
#define pin_Btn 3 // Кнопка

unsigned long CurrentTime, PrevEncoderTime;
enum eEncoderState
{
  eNone,
  eLeft,
  eRight,
  eButton
};
eEncoderState EncoderState;
int EncoderA, EncoderB, EncoderAPrev, counter;
bool ButtonPrev;

// ********** Прототипы функций ***************
eEncoderState GetEncoderState();
void LCDBacklight(byte v = 2);
eMenuKey DrawMenu(eMenuKey Key);

// ********** Обработчики для пунктов меню **********
int InputValue(char *Title, int DefaultValue, int MinValue, int MaxValue)
{
  // Вспомогательная функция для ввода значения
  lcd.clear();
  lcd.print(Title);
  lcd.setCursor(0, 1);
  lcd.print(DefaultValue);
  delay(100);
  while (1)
  {
    EncoderState = GetEncoderState();
    switch (EncoderState)
    {
    case eNone:
    {
      LCDBacklight();
      continue;
    }
    case eButton:
    {
      LCDBacklight(1);
      return DefaultValue;
    }
    case eLeft:
    {
      LCDBacklight(1);
      if (DefaultValue > MinValue)
        DefaultValue--;
      else
        DefaultValue = MaxValue;
      break;
    }
    case eRight:
    {
      LCDBacklight(1);
      if (DefaultValue < MaxValue)
        DefaultValue++;
      else
        DefaultValue = MinValue;
      break;
    }
    }
    lcd.setCursor(0, 1);
    lcd.print(Blank);
    lcd.setCursor(0, 1);
    lcd.print(DefaultValue);
  }
};

void writeTimerMenu()
{
  lcd.setCursor(2, 1);
  lcd.print("Period");
  lcd.setCursor(10, 1);
  lcd.print("Time");
  lcd.setCursor(15, 1);
  lcd.print("[sec]");
};

void writeAutoManMenu(bool parameter)
{
  if (parameter)
  { // true
    lcd.setCursor(2, 0);
    lcd.print("Automatic ON");
    lcd.setCursor(2, 3);
    lcd.print("Back");
  }
  else
  { // false
    lcd.setCursor(2, 0);
    lcd.print("Automatic OFF");
    lcd.setCursor(2, 3);
    lcd.print("Back");
    writeTimerMenu();
  }
};
void turnOnAuto(int arrPosition)
{
  switch (arrPosition)
  {
  case 0:
  {
    lcd.setCursor(0, 0);
    lcd.print("->");
    break;
  }
  case 1:
  {
    lcd.setCursor(0, 4);
    lcd.print("->");
    break;
  }
  }
};
void turnOffAuto(int arrPosition)
{
  switch (arrPosition)
  {
  case 0:
  {
    lcd.setCursor(0, 0); // mode
    lcd.print("->");
    break;
  }
  case 1:
  {
    lcd.setCursor(0, 1); // period
    lcd.print("->");
    break;
  }
  case 2:
  {
    lcd.setCursor(8, 1); // time
    lcd.print("->");
    break;
  }
  case 3:
  {
    lcd.setCursor(0, 3); // back
    lcd.print("->");
    break;
  }
  }
};
void arrowPosition(int arrPos, bool autoManual)
{
  if (autoManual)
  { // true
    turnOnAuto(arrPos);
  }
  else
  { // false
    turnOffAuto(arrPos);
  };
};
void additionalDataWrite(byte *data1, byte *data2)
{
  lcd.setCursor(2, 2);
  lcd.print(*data1);
  lcd.setCursor(10, 2);
  lcd.print(*data2);
};
void AutomaticManual()
{
  bool modeChoise = false;
  bool periodChoise = false;
  bool timerChoise = false;

  byte periodTime = 0;
  byte waterOnTime = 0;

  // bool autoManual = false; //auto true - manual false
  int choiseMenu = 0;
  lcd.clear();

  writeAutoManMenu(autoManual);

  arrowPosition(choiseMenu, autoManual);
  while (1)
  {
    lcd.setCursor(19, 0);
    lcd.print(choiseMenu);

    if (autoManual)
    {
      if (choiseMenu)
      {
        lcd.setCursor(0, 3); // back
        lcd.print("->");
        lcd.setCursor(0, 0);
        lcd.print("  ");
      }
      else
      {
        lcd.setCursor(0, 0); // choise
        lcd.print("->");
      };
      // return;
    }; // 4to budet kogda zaidu w sled raz

    EncoderState = GetEncoderState();
    switch (EncoderState)
    {
    case eNone:
    {
      LCDBacklight();

      continue;
    }
    case eButton:
    {
      LCDBacklight(1);
      if (choiseMenu == 0 && modeChoise == false)
      {
        modeChoise = true;
        break;
      };
      if (choiseMenu == 0 && modeChoise == true)
      {
        modeChoise = false;
        break;
      };
      if (autoManual == false)
      { // manual mode
        if (choiseMenu == 1 && periodChoise == false)
        {
          periodChoise = true;
          break;
        };
        if (choiseMenu == 1 && periodChoise == true)
        {
          periodChoise = false;
          break;
        }; // zapisat izmeneni9 period
        if (choiseMenu == 2 && timerChoise == false)
        {
          timerChoise = true;
          break;
        };
        if (choiseMenu == 2 && timerChoise == true)
        {
          timerChoise = false;
          break;
        }; // zapisat izmeneni9 time
           //}else{
        if (choiseMenu == 3)
        {
          return;
        }; //=========================================================?
      };
      if (autoManual && choiseMenu)
      {
        return;
      };
      // zapisat w EEPROM nastroiki
      // if(AutoManual){
      //  }else
      break;
    }
    case eLeft:
    {
      lcd.clear();
      LCDBacklight(1);
      if (modeChoise)
        autoManual = false;
      if (!modeChoise && !periodChoise && !timerChoise)
      { // esli net modow dla izmenenii
        if (choiseMenu <= 0)
          choiseMenu = 0;
        else
          choiseMenu--;
      }
      else
      {
        // proverka kogo izmenit nuzno
        if (periodChoise == true)
        {
          if (periodTime < 1)
            periodTime = 255;
          else
            periodTime--;
        };
        if (timerChoise == true)
        {
          if (waterOnTime < 1)
            waterOnTime = 255;
          else
            waterOnTime--;
        };
      };

      break;
    }
    case eRight:
    {
      lcd.clear();
      LCDBacklight(1);
      if (modeChoise)
        autoManual = true;
      if (!modeChoise && !periodChoise && !timerChoise)
      {

        if (autoManual)
        {
          if (choiseMenu >= 1)
            choiseMenu = 1;
          else
            choiseMenu++;
        }
        else
        {
          if (choiseMenu >= 3)
            choiseMenu = 3;
          else
            choiseMenu++;
        }
      };

      // else{
      if (periodChoise == true)
      { // +- 1 hour, byte ? 0-255
        if (periodTime > 254)
          periodTime = 1;
        else
          periodTime++;
      };
      if (timerChoise == true)
      { // +- 1 sec, byte ? 0-255
        if (waterOnTime > 254)
          waterOnTime = 1;
        else
          waterOnTime++;
      };
      //};
      break;
    }
    }

    writeAutoManMenu(autoManual);
    // arrowPosition(choiseMenu,autoManual);
    if (periodChoise || timerChoise)
    {
      additionalDataWrite(&periodTime, &waterOnTime);
    }
    else
    {
      arrowPosition(choiseMenu, autoManual);
    };
  }
};

void FrelayManager(byte numOfRelay)
{
  switch (numOfRelay)
  {
  case 1:
    // digitalWrite(PIN_RELAY1, HIGH);
    digitalWrite(PIN_RELAY2, HIGH);
    digitalWrite(PIN_RELAY3, HIGH);
    digitalWrite(PIN_RELAY4, HIGH);
    break;
  // case 1:
  // digitalWrite(PIN_RELAY1, LOW);
  // digitalWrite(PIN_RELAY2, HIGH);
  // digitalWrite(PIN_RELAY3, HIGH);
  // digitalWrite(PIN_RELAY4, HIGH);
  // break;
  case 2:
    // digitalWrite(PIN_RELAY1, HIGH);
    digitalWrite(PIN_RELAY2, LOW);
    digitalWrite(PIN_RELAY3, HIGH);
    digitalWrite(PIN_RELAY4, HIGH);
    break;
  case 3:
    // digitalWrite(PIN_RELAY1, HIGH);
    digitalWrite(PIN_RELAY2, HIGH);
    digitalWrite(PIN_RELAY3, LOW);
    digitalWrite(PIN_RELAY4, HIGH);
    break;
  case 4:
    // digitalWrite(PIN_RELAY1, HIGH);
    digitalWrite(PIN_RELAY2, HIGH);
    digitalWrite(PIN_RELAY3, HIGH);
    digitalWrite(PIN_RELAY4, LOW);
    break;
  }
}
void allPositions()
{
  lcd.setCursor(2, 0);
  lcd.print("All off ");
  lcd.setCursor(2, 1);
  lcd.print("Green ");
  lcd.setCursor(2, 2);
  lcd.print("Red ");
  lcd.setCursor(2, 3);
  lcd.print("Yellow ");
  lcd.setCursor(16, 3);
  lcd.print("Back ");
}
void offOnShow(byte Pos, bool offOn)
{
  lcd.setCursor(9, Pos - 1);
  if (offOn)
  {
    lcd.print("true");
    FrelayManager(Pos);
  }
  else
  {
    lcd.print("false");
    FrelayManager(1);
  };
}
void Frelay()
{
  byte choise = 1;
  bool choiseFlag = false;
  bool offOn = false;
  lcd.clear();
  allPositions();
  while (1)
  {
    // lcd.setCursor(2,0);
    // lcd.print("Relay ");
    // lcd.setCursor(8,0);
    // lcd.print(choise);
    switch (choise)
    {
    case 1:
    {
      lcd.setCursor(0, 0);
      lcd.print("->");
      if (choiseFlag)
      {
        offOnShow(choise, offOn);
      };
      break;
    }
    case 2:
    {
      lcd.setCursor(0, 1);
      lcd.print("->");
      if (choiseFlag)
      {
        offOnShow(choise, offOn);
      };
      break;
    }
    case 3:
    {
      lcd.setCursor(0, 2);
      lcd.print("->");
      if (choiseFlag)
      {
        offOnShow(choise, offOn);
      };
      break;
    }
    case 4:
    {
      lcd.setCursor(0, 3);
      lcd.print("->");
      if (choiseFlag)
      {
        offOnShow(choise, offOn);
      };
      break;
    }
    case 5:
    {
      lcd.setCursor(14, 3);
      lcd.print("->");
      break;
    }
    }
    EncoderState = GetEncoderState();
    switch (EncoderState)
    {
    case eNone:
    {
      LCDBacklight();
      continue;
    }
    case eButton:
    {
      LCDBacklight(1);
      if (choise == 5 && !choiseFlag)
      {
        return;
      };
      if ((choise > 0 && choise < 5) && !choiseFlag)
      {
        choiseFlag = true;
        break;
      };
      if ((choise > 0 && choise < 5) && choiseFlag)
      {
        choiseFlag = false;
        break;
      };
      // break;
      // return;
    }
    case eLeft:
    {
      lcd.clear();
      allPositions();
      LCDBacklight(1);
      if (choiseFlag)
      {
        offOn = true;
        break;
      }; // jesli stoit modifikacija to men9em znaczenie pol9
      if (choise <= 1)
        choise = 1;
      else
        choise--;
      // FrelayManager(choise);

      break;
    }
    case eRight:
    {
      lcd.clear();
      allPositions();
      LCDBacklight(1);
      if (choiseFlag)
      {
        offOn = false;
        break;
      }; // jesli stoit modifikacija to men9em znaczenie pol9
      if (choise >= 5)
        choise = 5;
      else
        choise++;
      // FrelayManager(choise);

      break;
    }
    }
  }
};

void FsensorBH1750()
{
  myTimer1 = millis();
  lcd.clear();

  sensorBH1750.begin();
  lcd.setCursor(0, 0);
  lcd.print("Sensor is loading");
  lcd.setCursor(0, 1);
  lcd.print("It will take 2 sec.");

  while (1)
  {
    EncoderState = GetEncoderState();
    switch (EncoderState)
    {
    case eNone:
    {
      LCDBacklight();

      if (millis() - myTimer1 > 2000)
      {
        lcd.clear();
        myTimer1 = millis();
        uint16_t lux = sensorBH1750.readLightLevel();
        lcd.print("Light: ");
        lcd.setCursor(0, 1);
        lcd.print(lux);
        lcd.setCursor(0, 2);
        lcd.print("lux");
      }
      continue;
    }
    case eButton:
    {
      LCDBacklight(1);
      return;
    }
    case eRight:
    {
      LCDBacklight(1);
    }
    case eLeft:
    {
      LCDBacklight(1);
    }
    }
  }
};

void DS18B20()
{
  myTimer1 = millis();
  lcd.clear();
  sensorDS18B20.requestTemp();
  lcd.setCursor(0, 0);
  lcd.print("Sensor is loading");
  lcd.setCursor(0, 1);
  lcd.print("It will take 2 sec.");

  while (1)
  {
    EncoderState = GetEncoderState();
    switch (EncoderState)
    {
    case eNone:
    {
      LCDBacklight();

      if (millis() - myTimer1 > 2000)
      {
        lcd.clear();
        myTimer1 = millis();
        sensorDS18B20.requestTemp();
        if (sensorDS18B20.readTemp())
          lcd.print(sensorDS18B20.getTemp());
        else
          lcd.print("error");
      }
      continue;
    }
    case eButton:
    {
      LCDBacklight(1);
      return;
    }

    case eRight:
    {
      LCDBacklight(1);
    }
    case eLeft:
    {
      LCDBacklight(1);
    }
    }
  }
};

void DHT11()
{
  myTimer1 = millis();
  lcd.clear();
  int chk;

  lcd.setCursor(0, 0);
  lcd.print("Sensor is loading");
  lcd.setCursor(0, 1);
  lcd.print("It will take 2 sec.");

  while (1)
  {
    EncoderState = GetEncoderState();
    switch (EncoderState)
    {
    case eNone:
    {
      LCDBacklight();

      if (millis() - myTimer1 > 2000)
      {
        lcd.clear();
        myTimer1 = millis();
        chk = DHT.read11(DHT11_PIN);
        switch (chk)
        {
        case DHTLIB_OK:
          lcd.print("OK");
          break;
        case DHTLIB_ERROR_CHECKSUM:
          lcd.print("Checksum error");
          break;
        case DHTLIB_ERROR_TIMEOUT:
          lcd.print("Time out error");
          break;
        default:
          lcd.print("Unknown error");
          break;
        }
        lcd.setCursor(0, 1);
        lcd.print("Wilgotn. HR [%]");
        lcd.setCursor(15, 1);
        lcd.print(DHT.getHumidity());
        lcd.setCursor(0, 2);
        lcd.print("Temperatura [C]");
        lcd.setCursor(15, 2);
        lcd.print(DHT.getTemperature());
      }

      continue;
    }
    case eButton:
    {
      LCDBacklight(1);
      return;
    }
    case eRight:
    {
      LCDBacklight(1);
    }
    case eLeft:
    {
      LCDBacklight(1);
    }
    }
  }
};

void CapMenuKalibration(int parameters[], int choiseParam)
{
  lcd.setCursor(2, 0);
  lcd.print("Suchy");
  lcd.setCursor(15, 0);
  lcd.print(parameters[0]);
  lcd.setCursor(2, 1);
  lcd.print("W wodzie");
  lcd.setCursor(15, 1);
  lcd.print(parameters[1]);
  lcd.setCursor(2, 2);
  lcd.print("W ziemi such.");
  lcd.setCursor(15, 2);
  lcd.print(parameters[2]);
  lcd.setCursor(2, 3);
  lcd.print("W ziemi mokr.");
  lcd.setCursor(15, 3);
  lcd.print(parameters[3]);
};

void CapSensor()
{
  // снимаем показание с сухого датчика (data1)
  // опускаем в воду                    (data2)
  //    верхний и нижний пределеы шкалы
  // погружаем в землю                  (data3)
  // полить                             (data4)
  lcd.clear();
  int val = analogRead(ANALOGA_IN);
  int numParameters = 4;
  int choise = 0;
  int exitFrom = 0;
  int parameters[numParameters] = {0, 0, 0, 0};
  unsigned long myTimerEnkLong = 0;

  if (calibrParameters[0] != 0 && calibrParameters[1] != 0 && calibrParameters[2] != 0 && calibrParameters[3] != 0)
  { // если есть уже данные в глобальном массиве то выводим то что есть
    for (int i = 0; i <= 3; i++)
    {
      parameters[i] = calibrParameters[i];
    }
  };

  while (1)
  {
    if (exitFrom == 4)
    {
      if (parameters[0] != 0 && parameters[1] != 0 && parameters[2] != 0 && parameters[3] != 0)
      { // если выкрутили выход и все значения заданы то пишем в глобальный массив (ЗАПИСЬ В EEPROM ОРГАНИЗОВАТЬ ОТДЕЛЬНО)
        for (int i = 0; i <= 3; i++)
        {
          calibrParameters[i] = parameters[i];
        }
      };
      return;
    };

    myTimerEnkLong = 0;
    lcd.setCursor(9, 0);
    lcd.print(choise);
    lcd.setCursor(11, 0);
    lcd.print(val);
    if (millis() - myTimer2 > 200)
    {
      myTimer2 = millis();
      val = analogRead(ANALOGA_IN);
      CapMenuKalibration(parameters, choise);
    }

    EncoderState = GetEncoderState();
    switch (EncoderState)
    {
    case eNone:
    {
      LCDBacklight();

      if (millis() - myTimer1 > 200)
      {
        myTimer1 = millis();
        // CapMenuKalibration(parameters, choise);
        lcd.setCursor(0, choise);
        lcd.print("->");
        // lcd.clear();
        //  opros dat4ika
      }
      // val = analogRead(ANALOGA_IN); // считываем данные
      // lcd.clear();
      // lcd.print(map(val,310,780,0,100));         // 317,318 - woda 810 - suhoi //val);//  78- suhoi 31 - woda
      // delay(150);
      continue;
    }
    case eButton:
    {
      LCDBacklight(1);
      parameters[choise] = val;
      //------------------------------------------
      // return;
      break;
    }
    case eRight:
    {
      LCDBacklight(1);
      exitFrom++;
      if (choise >= 3)
        choise = 3;
      else
      {
        choise++;
        lcd.clear();
      }
      break;
    }
    case eLeft:
    {
      LCDBacklight(1);
      exitFrom--;
      if (choise <= 0)
        choise = 0;
      else
      {
        choise--;
        lcd.clear();
      }
      break;
    }
    }

    lcd.setCursor(2, 0);
    lcd.print("Suchy");
    lcd.setCursor(15, 0);
    lcd.print(parameters[0]);
    lcd.setCursor(2, 1);
    lcd.print("W wodzie");
    lcd.setCursor(15, 1);
    lcd.print(parameters[1]);
    lcd.setCursor(2, 2);
    lcd.print("W ziemi such.");
    lcd.setCursor(15, 2);
    lcd.print(parameters[2]);
    lcd.setCursor(2, 3);
    lcd.print("W ziemi mokr.");
    lcd.setCursor(15, 3);
    lcd.print(parameters[3]);
  }
};
void realTime()
{
  lcd.clear();

  while (1)
  {
    EncoderState = GetEncoderState();
    switch (EncoderState)
    {
    case eNone:
    {
      LCDBacklight();

      printTime();

      continue;
    }
    case eButton:
    {
      LCDBacklight(1);
      return;
    }
    }
  }

  // delay (2000);
};

void SetClockData(Time data)
{
  rtc.writeProtect(false);
  rtc.halt(false);
  rtc.time(data);
};

void InputDzien()
{
  dzien = InputValue("Input dzien", dzien, 1, 31);
  while (GetEncoderState() == eNone)
    LCDBacklight();
  Time t(rok, miesiac, dzien, godziny, minuty, sekundy, Time::kSunday);
  SetClockData(t);
};
void InputMiesiac()
{
  miesiac = InputValue("Input miesiac", miesiac, 1, 12);
  while (GetEncoderState() == eNone)
    LCDBacklight();
  Time t(rok, miesiac, dzien, godziny, minuty, sekundy, Time::kSunday);
  SetClockData(t);
};
void InputRok()
{
  rok = InputValue("Input rok", rok, 2022, 2100);
  while (GetEncoderState() == eNone)
    LCDBacklight();
  Time t(rok, miesiac, dzien, godziny, minuty, sekundy, Time::kSunday);
  SetClockData(t);
};
void InputSec()
{
  sekundy = InputValue("Input sekundy", sekundy, 0, 59);
  while (GetEncoderState() == eNone)
    LCDBacklight();
  Time t(rok, miesiac, dzien, godziny, minuty, sekundy, Time::kSunday);
  SetClockData(t);
};
void InputMin()
{
  minuty = InputValue("Input minuty", minuty, 0, 59);
  while (GetEncoderState() == eNone)
    LCDBacklight();
  Time t(rok, miesiac, dzien, godziny, minuty, sekundy, Time::kSunday);
  SetClockData(t);
};
void InputHour()
{
  godziny = InputValue("Input godziny", godziny, 0, 23);
  while (GetEncoderState() == eNone)
    LCDBacklight();
  Time t(rok, miesiac, dzien, godziny, minuty, sekundy, Time::kSunday);
  SetClockData(t);
};

// ******************** Меню ********************
byte ScrollUp[8] = {0x4, 0xa, 0x11, 0x1f};
byte ScrollDown[8] = {0x0, 0x0, 0x0, 0x0, 0x1f, 0x11, 0xa, 0x4};

byte ItemsOnPage = rows; // Максимальное количество элементов для отображения на экране
unsigned long BacklightOffTime = 0;
unsigned long ScrollTime = 0;
byte ScrollPos;
byte CaptionMaxLength;

struct sMenuItem
{
  eMenuKey Parent;   // Ключ родителя
  eMenuKey Key;      // Ключ
  char *Caption;     // Название пункта меню
  void (*Handler)(); // Обработчик
};

sMenuItem Menu[] = {
    {mkNull, mkRoot, "Menu", NULL},
    {mkRoot, mkMenu1, "Time and Date", NULL},
    {mkMenu1, mkSettings, "Show Time and Date", realTime},
    {mkMenu1, mkBack, "Back", NULL},

    {mkRoot, mkMenu2, "Ustaiwenia zegaru", NULL},
    {mkMenu2, mkEnterDzien, "Dzien", InputDzien},
    {mkMenu2, mkEnterMiesiac, "Miesiac", InputMiesiac},
    {mkMenu2, mkEnterRok, "Rok", InputRok},
    {mkMenu2, mkEnterSec, "Sekundy", InputSec},
    {mkMenu2, mkEnterMin, "Minuty", InputMin},
    {mkMenu2, mkEnterHour, "Godziny", InputHour},
    {mkMenu2, mkBack, "Back", NULL},

    {mkRoot, mkMenu3, "TemperatureHumid.", NULL},
    {mkMenu3, mkDHT11, "DHT11", DHT11},
    {mkMenu3, mkBack, "Back", NULL},

    {mkRoot, mkMenu4, "CapacitySensor", NULL},
    {mkMenu4, mkCapacity, "CapacitySensor", CapSensor},
    {mkMenu4, mkBack, "Back", NULL},

    {mkRoot, mkMenu5, "GroundTemperature", NULL},
    {mkMenu5, mkGroundTemp, "DS18B20", DS18B20},
    {mkMenu5, mkBack, "Back", NULL},

    {mkRoot, mkMenu6, "LightMeter", NULL},
    {mkMenu6, mkLightMeter, "BH1750", FsensorBH1750},
    {mkMenu6, mkBack, "Back", NULL},

    {mkRoot, mkMenu7, "Relay", NULL},
    {mkMenu7, mkRelay, "ChoiceRelay", Frelay},
    {mkMenu7, mkBack, "Back", NULL},

    {mkRoot, mkMenu8, "AutomaticManual", NULL},
    {mkMenu8, mkAutomatManual, "AutomaticManual", AutomaticManual},
    {mkMenu8, mkBack, "Back", NULL},
};

const int MenuLength = sizeof(Menu) / sizeof(Menu[0]);

void LCDBacklight(byte v)
{ // Управление подсветкой
  if (v == 0)
  { // Выключить подсветку
    BacklightOffTime = millis();
    lcd.noBacklight();
  }
  else if (v == 1)
  { // Включить подсветку
    BacklightOffTime = millis() + BacklightDelay;
    lcd.backlight();
  }
  else
  { // Выключить если время вышло
    if (BacklightOffTime < millis())
      lcd.noBacklight();
    else
      lcd.backlight();
  }
}

eMenuKey DrawMenu(eMenuKey Key)
{ // Отрисовка указанного уровня меню и навигация по нему
  eMenuKey Result;
  int k, l, Offset, CursorPos, y;
  sMenuItem **SubMenu = NULL;
  bool NeedRepaint;
  String S;
  l = 0;
  LCDBacklight(1);
  // Запишем в SubMenu элементы подменю
  for (byte i = 0; i < MenuLength; i++)
  {
    if (Menu[i].Key == Key)
    {
      k = i;
    }
    else if (Menu[i].Parent == Key)
    {
      l++;
      SubMenu = (sMenuItem **)realloc(SubMenu, l * sizeof(void *));
      SubMenu[l - 1] = &Menu[i];
    }
  }

  if (l == 0)
  { // l==0 - подменю нет
    if ((ReturnFromMenu == 0) and (Menu[k].Handler != NULL))
      (*Menu[k].Handler)(); // Вызываем обработчик если он есть
    LCDBacklight(1);
    return Key; // и возвращаем индекс данного пункта меню
  }

  // Иначе рисуем подменю
  CursorPos = 0;
  Offset = 0;
  ScrollPos = 0;
  NeedRepaint = 1;
  do
  {
    if (NeedRepaint)
    {
      NeedRepaint = 0;
      lcd.clear();
      y = 0;
      for (int i = Offset; i < min(l, Offset + ItemsOnPage); i++)
      {
        lcd.setCursor(1, y++);
        lcd.print(String(SubMenu[i]->Caption).substring(0, CaptionMaxLength));
      }
      lcd.setCursor(0, CursorPos);
      lcd.print(">");
      if (ShowScrollBar)
      {
        if (Offset > 0)
        {
          lcd.setCursor(cols - 1, 0);
          lcd.write(0);
        }
        if (Offset + ItemsOnPage < l)
        {
          lcd.setCursor(cols - 1, ItemsOnPage - 1);
          lcd.write(1);
        }
      }
    }
    EncoderState = GetEncoderState();
    switch (EncoderState)
    {
    case eLeft:
    {
      // Прокрутка меню вверх
      LCDBacklight(1);
      ScrollTime = millis() + ScrollDelay * 5;
      if (CursorPos > 0)
      { // Если есть возможность, поднимаем курсор
        if ((ScrollLongCaptions) and (ScrollPos))
        {
          // Если предыдущий пункт меню прокручивался, то выводим его заново
          lcd.setCursor(1, CursorPos);
          lcd.print(Blank);
          lcd.setCursor(1, CursorPos);
          lcd.print(String(SubMenu[Offset + CursorPos]->Caption).substring(0, CaptionMaxLength));
          ScrollPos = 0;
        }
        // Стираем курсор на старом месте, рисуем в новом
        lcd.setCursor(0, CursorPos--);
        lcd.print(" ");
        lcd.setCursor(0, CursorPos);
        lcd.print(">");
      }
      else if (Offset > 0)
      {
        // Курсор уже в крайнем положении. Если есть пункты выше, то перерисовываем меню
        Offset--;
        NeedRepaint = 1;
      }
      break;
    }
    case eRight:
    {
      // Прокрутка меню вниз
      LCDBacklight(1);
      ScrollTime = millis() + ScrollDelay * 5;
      if (CursorPos < min(l, ItemsOnPage) - 1)
      { // Если есть возможность, то опускаем курсор
        if ((ScrollLongCaptions) and (ScrollPos))
        {
          // Если предыдущий пункт меню прокручивался, то выводим его заново
          lcd.setCursor(1, CursorPos);
          lcd.print(Blank);
          lcd.setCursor(1, CursorPos);
          lcd.print(String(SubMenu[Offset + CursorPos]->Caption).substring(0, CaptionMaxLength));
          ScrollPos = 0;
        }
        // Стираем курсор на старом месте, рисуем в новом
        lcd.setCursor(0, CursorPos++);
        lcd.print(" ");
        lcd.setCursor(0, CursorPos);
        lcd.print(">");
      }
      else
      {
        // Курсор уже в крайнем положении. Если есть пункты ниже, то перерисовываем меню
        if (Offset + CursorPos + 1 < l)
        {
          Offset++;
          NeedRepaint = 1;
        }
      }
      break;
    }
    case eButton:
    {
      // Выбран элемент меню. Нажатие кнопки Назад обрабатываем отдельно
      LCDBacklight(1);
      ScrollTime = millis() + ScrollDelay * 5;
      if (SubMenu[CursorPos + Offset]->Key == mkBack)
      {
        free(SubMenu);
        return mkBack;
      }
      Result = DrawMenu(SubMenu[CursorPos + Offset]->Key);
      if ((Result != mkBack) and (ReturnFromMenu))
      {
        free(SubMenu);
        return Result;
      }
      NeedRepaint = 1;
      break;
    }
    case eNone:
    {
      if (ScrollLongCaptions)
      {
        // При бездействии прокручиваем длинные названия
        S = SubMenu[CursorPos + Offset]->Caption;
        if (S.length() > CaptionMaxLength)
        {
          if (ScrollTime < millis())
          {
            ScrollPos++;
            if (ScrollPos == S.length() - CaptionMaxLength)
              ScrollTime = millis() + ScrollDelay * 2; // Небольшая задержка когда вывели все название
            else if (ScrollPos > S.length() - CaptionMaxLength)
            {
              ScrollPos = 0;
              ScrollTime = millis() + ScrollDelay * 5; // Задержка перед началом прокрутки
            }
            else
              ScrollTime = millis() + ScrollDelay;
            lcd.setCursor(1, CursorPos);
            lcd.print(Blank);
            lcd.setCursor(1, CursorPos);
            lcd.print(S.substring(ScrollPos, ScrollPos + CaptionMaxLength));
          }
        }
      }
      LCDBacklight();
    }
    }
  } while (1);
}
//****************************************
void DrawFirst()
{
  lcd.setCursor(0, 0);
  lcd.print("Zborowski Miroslaw");
  lcd.setCursor(0, 1);
  lcd.print("SGGW nr.199543");
  lcd.setCursor(0, 2);
  lcd.print("     Zadowolony     ");
  lcd.setCursor(0, 3);
  lcd.print("     Piri-Piri     ");
  delay(4000);
}
void setup()
{
  // relay

  // digitalWrite(PIN_RELAY1, HIGH); // Выключаем реле - посылаем высокий сигнал //13
  // pinMode(PIN_RELAY1, OUTPUT); // Объявляем пин реле как выход

  digitalWrite(PIN_RELAY2, HIGH); // Выключаем реле - посылаем высокий сигнал // 8
  pinMode(PIN_RELAY2, OUTPUT);    // Объявляем пин реле как выход

  digitalWrite(PIN_RELAY3, HIGH); // Выключаем реле - посылаем высокий сигнал // 7
  pinMode(PIN_RELAY3, OUTPUT);    // Объявляем пин реле как выход

  digitalWrite(PIN_RELAY4, HIGH); // Выключаем реле - посылаем высокий сигнал // 6
  pinMode(PIN_RELAY4, OUTPUT);    // Объявляем пин реле как выход
                                  // relay

  //------------ds1302
  // rtc.writeProtect(false);
  // rtc.halt(false);
  lastTimeOn(&dzien, &miesiac, &rok, &sekundy, &minuty, &godziny);
  //------------ds1302

  pinMode(pin_CLK, INPUT);
  pinMode(pin_DT, INPUT);
  pinMode(pin_Btn, INPUT_PULLUP);
  lcd.begin();
  lcd.backlight();
  CaptionMaxLength = cols - 1;
  Blank = (char *)malloc(cols * sizeof(char));
  for (byte i = 0; i < CaptionMaxLength; i++)
    Blank[i] = ' ';
  if (ShowScrollBar)
  {
    CaptionMaxLength--;
    lcd.createChar(0, ScrollUp);
    lcd.createChar(1, ScrollDown);
  }
  Blank[CaptionMaxLength] = 0;

  //------------dht11
  // lcd.createChar(1, gradus);
  // dht.begin();
  //------------dht11

  // zapusk 1 lub perezagruszka
  DrawFirst();
}

void loop()
{
  DrawMenu(mkRoot);
}

// ******************** Энкодер с кнопкой ********************
eEncoderState GetEncoderState()
{
  // Считываем состояние энкодера
  eEncoderState Result = eNone;
  CurrentTime = millis();
  if (CurrentTime >= (PrevEncoderTime + 5))
  {
    PrevEncoderTime = CurrentTime;
    if (digitalRead(pin_Btn) == LOW)
    {
      if (ButtonPrev)
      {
        Result = eButton; // Нажата кнопка
        ButtonPrev = 0;
      }
    }
    else
    {
      ButtonPrev = 1;
      EncoderA = digitalRead(pin_DT);
      EncoderB = digitalRead(pin_CLK);
      if ((!EncoderA) && (EncoderAPrev))
      { // Сигнал A изменился с 1 на 0
        if (EncoderB)
          Result = eRight; // B=1 => энкодер вращается по часовой
        else
          Result = eLeft; // B=0 => энкодер вращается против часовой
      }
      EncoderAPrev = EncoderA; // запомним текущее состояние
    }
  }
  return Result;
}
