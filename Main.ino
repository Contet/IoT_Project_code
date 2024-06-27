#include <BaseMQ.h>
#include <MQ135.h>
#include <MQ2.h>
#include <MQ3.h>
#include <MQ4.h>
#include <MQ5.h>
#include <MQ6.h>
#include <MQ7.h>
#include <MQ8.h>
#include <MQ9.h>
#include <TroykaMQ.h>

#include <TroykaMeteoSensor.h>
#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>

#include <TroykaCurrent.h>

#include <SoftwareSerial.h>

Thread timerModeThread = Thread();
Thread ledThread = Thread();
Thread calibrateThread = Thread();
Thread positionThread = Thread();
Thread calibrate_currentThread = Thread();
Thread a_timerModeThread = Thread();

TroykaMeteoSensor meteoSensor;

Thread meteoSensorThread = Thread();


Thread bluetoothThread = Thread();

#include <string>

const int averageValue = 500;       // Переменная для хранения значения количества считывания циклов 
long int sensorValue = 0;           // Переменная для хранения значения с датчика

float voltage = 0;                  // Переменная для хранения значения напряжения
float current = 0.5;   


#define MAX_DIGITS 10
char digitArray[MAX_DIGITS]; // Массив для хранения цифр
int currentIndex = 0;


ACS712 sensorCurrent(A5);
SoftwareSerial BLEPIN(2, 3);
 
#define HC05_SERIAL BLEPIN
#define sensorpin D12
#define relepin 
#define onMotorPin 
#define offMotorPin


int temp_mas[100];
int humi_mas[100];

int humi_sr;
int temp_sr;
int value_humi;
int value_temp;
int value_processing;
int value_delay;
int n;
int sensor_flag;


String readValue;
int check_value;

const int chunkSize = 10;

float timer_counter;
float value_timer_position;
float value_timer_delayOFF;
float value_timer_delayON;
int flag_check;

bool timer_bool;

float calibrate_time;

float position_now;
float value_position;
float need_pos;
float start_position;

float check_time;

float change_pos;

// Важность 
float imp_temp;
float imp_humi; 

int g = 1;
void setup() {

  calibrateThread.onRun(calibrate); 
  calibrateThread.setInterval(1); 
  
  positionThread.onRun(position_main); 
  positionThread.setInterval(1); 

  calibrate_currentThread.onRun(current_th);
  calibrate_currentThread.setInterval(2);
  
  timerModeThread.onRun(timer_self);
  timerModeThread.setInterval(1);

  meteoSensorThread.onRun(sensor);
  meteoSensorThread.setInterval(1000);

  a_timerModeThread.onRun(auto_check);
  a_timerModeThread.setInterval(1);  

  pinMode(sensorpin, OUTPUT);
  // pinMode(relepin, OUTPUT);

  pinMode(D8, OUTPUT) ;
  pinMode(D10, OUTPUT) ;
  pinMode(D9, OUTPUT) ;

  meteoSensor.begin();

  Serial.begin(9600);
  
  Serial.println("Enter AT commands:");

  // bluetoothThread.onRun();
  HC05_SERIAL.begin(9600);


  while(!Serial) {
  }
  // печатаем сообщение об успешной инициализации Serial-порта
  Serial.println("Serial init OK");
  // начало работы с датчиком
  meteoSensor.begin();
  Serial.println("Meteo Sensor init OK");
  // ждём одну секунду
  delay(1000);
  

  // Параметры по умолчанию

  n = 0;
  value_humi = 50;
  value_temp = 24;
  value_processing = 30;
  value_delay = 30;
  
  value_position = 0.0;

  need_pos = 0;

  position_now = 0.0;

  timer_counter = 0.0;
  value_timer_delayOFF = 5.0;
  value_timer_delayON = 5.0;
  value_timer_position = 0.5;
  flag_check = 1;
  calibrate_time = 0.0;

  imp_temp = 1.0;
  imp_humi = 0.5;

  change_pos = 0;

  start_position = 0.5;

  main_menu();
}
void loop() {

}


// Главное меню

void main_menu(){
  int i;
  i = 1;

  HC05_SERIAL.write("Main menu:           ");
  HC05_SERIAL.write("Select operating mode:                  ");
  HC05_SERIAL.write("1.Automatic control ");
  HC05_SERIAL.write("2.Position control  ");
  HC05_SERIAL.write("3.Set timer         ");
  HC05_SERIAL.write("4.Calibrate device ");
  HC05_SERIAL.write("5.Manual control ");
  while(i > 0){

    if (HC05_SERIAL.available()){
    
        Serial.println("Увидела что активно avalibale");
        check_value = 0;
        readValue = HC05_SERIAL.read();
        check_value = atoi (readValue.c_str());

        char ch = static_cast<char>(check_value);

        Serial.println(ch);

        if (ch == '1'){
        Serial.println(HC05_SERIAL.read());
        Serial.println("Зашла в 1.");
        automatical_menu();
        i = 0;
      }
        if (ch == '2'){
        Serial.println(HC05_SERIAL.read());
        Serial.println("Зашла в 2.");
        manualControl_menu();
        i = 0;
      }
        if (ch == '3'){
        Serial.println(HC05_SERIAL.read());
        Serial.println("Зашла в 3.");
        timer_menu();
        i = 0;
      }
        if (ch == '4'){
        Serial.println(HC05_SERIAL.read());
        Serial.println("Зашла в 4.");
        calibrate_menu();
        i = 0;
      }
        if (ch == '5'){
        Serial.println(HC05_SERIAL.read());
        Serial.println("Зашла в 5.");
        i = 0;        
        o_c();
      }
        Serial.println(HC05_SERIAL.read());
        Serial.println("Полностью прошла avalibale");
    }
  }
  // main_menu();
}



void o_c(){

  int i;
  i = 1;


  HC05_SERIAL.write("Manual control menu ");
  HC05_SERIAL.write("1. Open             ");
  HC05_SERIAL.write("2. Close            ");
  HC05_SERIAL.write("3. Stop             ");
  HC05_SERIAL.write("5. Main menu ");
  while(i > 0){
   if (HC05_SERIAL.available()){
    
        check_value = 0;
        readValue = HC05_SERIAL.read();
        check_value = atoi (readValue.c_str());

        char ch = static_cast<char>(check_value);
   
      if (ch == '2'){
        Serial.println("Done");
        digitalWrite(D9, 0);
        digitalWrite(D8, 1);
        digitalWrite(D10, 1);
      }
      if (ch == '1'){
        Serial.println("Done");
        digitalWrite(D8, 0);
        digitalWrite(D9, 1);
        digitalWrite(D10, 1);
      }
      if (ch == '3'){
        Serial.println("Done");
        digitalWrite(D8, 0);
        digitalWrite(D9, 0);
        digitalWrite(D10, 0);
      }
      if (ch == '5'){
        i = 0;
        main_menu();
      }
    }
  }
  // o_c();
}
// Меню автоматического управления

void automatical_menu(){
  int i;
  i = 1;
  HC05_SERIAL.write("Automatic control me  nu:               ");
  HC05_SERIAL.write("1. Start.           ");
  HC05_SERIAL.write("2. Configure parameters.                ");
  HC05_SERIAL.write("3. Exit to the main menu                ");
  while(i>0){
    if (HC05_SERIAL.available()){
      check_value = 0;
      readValue = HC05_SERIAL.read();
      check_value = atoi (readValue.c_str());

      char ch = static_cast<char>(check_value);

        if (ch == '1'){
        automatical_mode();
      }



        if (ch == '2'){
        HC05_SERIAL.write("Temperature:        ");
        while(i == 1)

        if (HC05_SERIAL.available()){
        check_value = 0;
          readValue = HC05_SERIAL.read();
          check_value = atoi (readValue.c_str());
          char ch = static_cast<char>(check_value);

          addDigit(ch);
          int res = combineDigitsToInt();

            if(res > 10){
              value_temp = res;
              clearDigitArray();
              i = 2;
            }
            
          }
        }

        HC05_SERIAL.write("Humidity: ");
        while(i == 2){

        if (HC05_SERIAL.available()){
          check_value = 0;
          readValue = HC05_SERIAL.read();
          check_value = atoi (readValue.c_str());
          char ch = static_cast<char>(check_value);

          addDigit(ch);
          int res = combineDigitsToInt();

            if(res > 10){
              value_humi = res;
              clearDigitArray();
              i = 3;
            }

          }
        }

        HC05_SERIAL.write("OFF-time:           ");
        while(i == 3){
        
        if (HC05_SERIAL.available()){
          check_value = 0;
          readValue = HC05_SERIAL.read();
          check_value = atoi (readValue.c_str());
          char ch = static_cast<char>(check_value);

          addDigit(ch);
          int res = combineDigitsToInt();

          if(res > 1){
            value_timer_delayOFF = res;
            clearDigitArray();
            i = 4;
          }

          }
        } 
        HC05_SERIAL.write("ON-time:            ");
        while(i == 4){
        
        if (HC05_SERIAL.available()){
          check_value = 0;
          readValue = HC05_SERIAL.read();
          check_value = atoi (readValue.c_str());
          char ch = static_cast<char>(check_value);

          addDigit(ch);
          int res = combineDigitsToInt();

            if(res > 1){
              value_timer_delayON = res;
              clearDigitArray();
              automatical_menu();
            }

          }
        }
      



            if (ch == '3'){
            main_menu();
          }
      }
    }
  }


// Автоматический режим ( демо )

void automatical_mode(){
  bool aut;
  aut = true;
  timer_bool = true;
    HC05_SERIAL.write("1.Exit              ");

    while(aut){
      if (HC05_SERIAL.available()){
      check_value = 0;
      readValue = HC05_SERIAL.read();
      check_value = atoi (readValue.c_str());
      char ch = static_cast<char>(check_value);

      if(ch == '1') { timer_bool = false; aut = false; automatical_menu();}
      }

    if (timer_bool){
      if (a_timerModeThread.shouldRun()){
        a_timerModeThread.run(); 
        }
      } 

      // if(){
        
      // }

    }
    
}



void sensor(){

  int q = meteoSensor.read();
  int humi = meteoSensor.getHumidity();
  int humi_all = 0;
  int temp = meteoSensor.getTemperatureC();
  int temp_all = 0;

  // выводим инфу с датчиков
  Serial.print(" Temperature = ");
  Serial.print(temp - 10);
  Serial.println(" C ");
  temp_mas[n] = temp - 10;

  Serial.print(" Humidity = ");
  Serial.print(humi);
  Serial.println(" % ");
  humi_mas[n] = humi;

  Serial.println("Записанные температура : "); 
  for(int i = 0; i <= n; i++){
    Serial.print(temp_mas[i]);
    Serial.print(" , ");
    temp_all+=temp_mas[i];
  }

 Serial.println("");

  temp_sr = temp_all / (n + 1);

  Serial.print("Общее temp : ");
  Serial.println(temp_all);

  Serial.println("Среднее значение температуры : ");
  Serial.println(temp_sr);

  Serial.println("Записанные влажность : ");
  for(int i = 0; i <= n; i++){
    Serial.print(humi_mas[i]);
    Serial.print(" , ");
    humi_all+=humi_mas[i];
  }

  Serial.println("");

  humi_sr = humi_all / (n + 1);
  Serial.print("Общее humi : ");
  Serial.println(humi_all);
  Serial.println("Среднее значение влажности : ");
  Serial.println(humi_sr);

  n+=1;

}




// Таймер автоматического режима
void auto_check(){

  Serial.println("Таймер работает");
  Serial.println("Таймер насчитал : ");
  Serial.println(timer_counter);

  timer_counter = timer_counter + 0.1;

  if(flag_check == 2){

    Serial.println("Вошли во второй флаг");

    if(timer_counter > value_timer_delayOFF){

      timer_bool = false;


      value_position = 0.5;
      while(position_now != value_position){
        
        if(positionThread.shouldRun()){
          position_main();
        }

      }

    while(n < value_timer_delayON){

      if(meteoSensorThread.shouldRun()){
        meteoSensorThread.run();
        Serial.println("Значение n:  ");
        Serial.println(n);
        }
    }
      
      change_auto();
      flag_check = 3;
      timer_counter = 0.0;
      timer_bool = true;

    }
  }

  if(flag_check == 1){
    Serial.println("Вошли в первый флаг");
    
    
    if(timer_counter > value_timer_delayOFF){

      Serial.println("Таймер должен закрывать сейчас окно");

      timer_bool = false;

      while(position_now != 0.0){

        value_position = 0.0;
        
        if(positionThread.shouldRun()){
          position_main();
        }
        
      }

      while(n < value_timer_delayON){
        if(meteoSensorThread.shouldRun()){
          meteoSensorThread.run();
         Serial.println("Значение n:  ");
         Serial.println(n);
          
      }

      flag_check = 2;
      timer_counter = 0.0;
      timer_bool = true;


      }
    }
  }

if(flag_check == 3){
    Serial.println("Вошли в 3 флаг");
    
    
    if(timer_counter > value_timer_delayOFF){

      Serial.println("Таймер должен закрывать сейчас окно");

      timer_bool = false;

      while(position_now != start_position){

        value_position = start_position;
        
        if(positionThread.shouldRun()){
          position_main();
        }
        
      }

      while(n < value_timer_delayON){
        if(meteoSensorThread.shouldRun()){
          meteoSensorThread.run();
         Serial.println("Значение n:  ");
         Serial.println(n);
          
      }

      flag_check = 1;
      timer_counter = 0.0;
      timer_bool = true;


      }
    }
  }
  if(n == value_timer_delayON){
  n = 0;
  Serial.println("Значение n = 0 ");
  }

  // if (need_pos != position_now){
  //   value_position = need_pos;
  //       if(positionThread.shouldRun()){
  //     position_main();
  //   }
  // }

}

void change_auto(){
  if((temp_sr - value_temp > 0) && (start_position != 1)){
    if(temp_sr - value_temp > 2){
      Serial.println("Старт позиция изменилась на +0.1");
        start_position+=0.1;
      }
    }
  

  if(temp_sr - value_temp < 0){
    if((abs(temp_sr - value_temp) > 2) && (start_position != 0)){
      Serial.println("Старт позиция изменилась на -0.1");
        start_position-=0.1;
      }
    }
  

}

float charToFloat(char c) {
    // Проверка, является ли символ цифрой
    if (c >= '0' && c <= '9') {
        // Преобразование символа в соответствующее число
        return (float)(c - '0');
    } else {
        // Если символ не является цифрой, возвращаем 0.0 или можно использовать другую обработку ошибок
        return 0.0;
    }
}

// Меню позиционного управления

void manualControl_menu(){
  int i;
  i = 1;
  HC05_SERIAL.write("Position menu:       ");
  HC05_SERIAL.write("Enter a window position value in the range from 1 to 10 (for example: 5):           ");
  HC05_SERIAL.write("'e' for exit to menu ");
  while(i == 1){
    if (HC05_SERIAL.available()){
      check_value = 0;
      readValue = HC05_SERIAL.read();
      check_value = atoi (readValue.c_str());
      char ch = static_cast<char>(check_value);
      Serial.print(ch);

      float res = charToFloat(ch);
      // Serial.print(res);
      value_position = res / 10;

      // Serial.print(value_position);
      clearDigitArray();

      if(0.1 <= value_position <= 1.0 ){
          while( position_now != value_position){
            if (positionThread.shouldRun()){positionThread.run();}
          }
      
      }
      if(ch == 'e'){
        i = 0;
        main_menu();
      }
    }
  }

}

void clearDigitArray() {
    for (int i = 0; i < MAX_DIGITS; i++) {
        digitArray[i] = '\0'; // Обнуляем все элементы массива
    }
    currentIndex = 0; // Сбрасываем текущий индекс
}

// Функция для добавления цифры в массив
void addDigit(char digit) {
    if (currentIndex < MAX_DIGITS && digit >= '0' && digit <= '9') {
        digitArray[currentIndex] = digit;
        currentIndex++;
    } else {
        Serial.println("Ошибка: Либо массив переполнен, либо введен неверный символ");
        clearDigitArray();
    }
}

// Функция для объединения цифр и преобразования их в int
int combineDigitsToInt() {
    digitArray[currentIndex] = '\0'; // Завершаем строку нулевым символом
    int result = atoi(digitArray);    // Преобразуем строку в целое число
    return result;
}





// Меню таймера 

void timer_menu(){
  int i;
  i = 1;
  timer_counter = 0.0;
  
  HC05_SERIAL.write("You are in the timer control menu.        ");
 HC05_SERIAL.write("1. Set up timer      ");
 HC05_SERIAL.write("2. Start working in timer mode           ");
 HC05_SERIAL.write("3. Exit to the main menu                 ");
while(true){
  if (HC05_SERIAL.available()){
    check_value = 0;
    readValue = HC05_SERIAL.read();
    check_value = atoi (readValue.c_str());
    char ch = static_cast<char>(check_value);

    Serial.println(ch);
    // addDigit(ch);
    // int res = combineDigitsToInt();

    // Serial.println("Результат: ");
    // Serial.println(res);

    if(ch == '1'){
      HC05_SERIAL.write("OFF-time:           ");
      while(i == 1){
      if (HC05_SERIAL.available()){
        check_value = 0;
        readValue = HC05_SERIAL.read();
        check_value = atoi (readValue.c_str());

        ch = static_cast<char>(check_value);

        addDigit(ch);
        int res = combineDigitsToInt();
        Serial.println(res);
          if (res > 1){
            value_timer_delayOFF = res;
            i = 2;
            clearDigitArray();
          } 

        }
      }
      HC05_SERIAL.write("ON-time:            ");
      while(i == 2){
      if (HC05_SERIAL.available()){
        check_value = 0;
        readValue = HC05_SERIAL.read();
        check_value = atoi (readValue.c_str());

        char ch = static_cast<char>(check_value);

        addDigit(ch);
        int res = combineDigitsToInt();
        Serial.println(res);
          if (res > 1){
            value_timer_delayON = res;
            i = 3;
            clearDigitArray();
          }

        }
      }
      HC05_SERIAL.write("Enter a window position value in the range from 1 to 10 (for example: 5):           ");
      while(i == 3){
      if (HC05_SERIAL.available()){
        check_value = 0;
        readValue = HC05_SERIAL.read();
        check_value = atoi (readValue.c_str());
        
        char ch = static_cast<char>(check_value);

        addDigit(ch);
        int res = combineDigitsToInt();
        Serial.println(res);
          if (res / 10 <= 1){
            value_timer_position = res;
            clearDigitArray();
            timer_menu();
          }

        }
      }
    }

    if(ch == '2'){
      HC05_SERIAL.write("ON-time:            ");
      HC05_SERIAL.write("Timer is on         ");
      timer_mode();
    }

    if(ch == '3'){
      main_menu();
    }
  }
}
}







// Режим таймера
void timer_mode(){
  int i;
  i = 0;

  timer_bool = false;// Выключаем по умолчанию

  // устанавливаем окно в положение 0
  if(position_now != 0.0)
    while(position_now != 0.0){
      value_position = 0.0;
      if(positionThread.shouldRun()){
        position_main();
      }
    }
  else timer_bool = true; // Включаем 

  while(i == 0){
    HC05_SERIAL.write("Timer is on         ");
    HC05_SERIAL.write("1. Exit             ");

    while(i == 0 ){
      if (HC05_SERIAL.available()){
      check_value = 0;
      readValue = HC05_SERIAL.read();
      check_value = atoi (readValue.c_str());
      char ch = static_cast<char>(check_value);
      if(ch == '1') {i = 1; timer_menu();}
      }

    if (timer_bool){
      if (timerModeThread.shouldRun()){
        timerModeThread.run(); 
        }
      } 
    }
    
  }


}

// Таймер
void timer_self(){

  Serial.println("Таймер работает");
  Serial.println("Таймер насчитал : ");
  Serial.println(timer_counter);
  timer_counter = timer_counter + 0.1;

  if(flag_check == 1){
    if(timer_counter > value_timer_delayOFF){
      Serial.println("Таймер должен открывать сейчас окно");
      timer_bool = false;
      while(position_now != value_timer_position / 10){
        value_position = value_timer_position / 10;
        if(positionThread.shouldRun()){
          position_main();
        }
      }
      flag_check = 2;
      timer_counter = 0.0;
      timer_bool = true;
    }
  }
  if(flag_check == 2){
    Serial.println("Вошли во второй флаг");
    if(timer_counter > value_timer_delayON){
      Serial.println("Таймер должен закрывать сейчас окно");
      timer_bool = false;
      while(position_now != 0.0){
        value_position = 0.0;
        if(positionThread.shouldRun()){
          position_main();
        }
      }
      flag_check = 1;
      timer_counter = 0.0;
      timer_bool = true;
    }
  }
}









// Меню калибровки устройства
void current_th(){
  if(calibrate_time < 0.5)
    current = 0.5;
  
  else
  current = sensorCurrent.readCurrentAC();  
  Serial.println(current);
}

void calibrate_menu(){

  bool i = true;
  

  HC05_SERIAL.write("You are in the device calibration menu: ");
  HC05_SERIAL.write("Place the window in position 1 (full open) and enter the value '1'.             ");
  HC05_SERIAL.write("2. Exit             ");  
  while(i){ 
      if (HC05_SERIAL.available()){
      check_value = 0;
      readValue = HC05_SERIAL.read();
      check_value = atoi (readValue.c_str());
      char ch = static_cast<char>(check_value);

      if(ch == '1'){
        while( current > 0.06){
          if (calibrateThread.shouldRun()){
            calibrateThread.run(); 
          }
        }
      }

      if(ch == '2'){
        i = false;
        main_menu();
      }
    }
  }
}

// Калибровка

void calibrate(){
  calibrate_currentThread.run();

  // if (HC05_SERIAL.available()){
  //     check_value = 0;
  //     readValue = HC05_SERIAL.read();
  //     check_value = atoi (readValue.c_str());
  //     char ch = static_cast<char>(check_value);
  //     if(check_value == 1){
  //       current = 0.1;
  //     }
  // }
  // Пока значение силы тока меньше значения тока, которое увеличено в связи с нагрузкой
  if( current > 0.06){
    // закрутить
    digitalWrite(D9, 0);
    digitalWrite(D8, 1);
    digitalWrite(D10, 1);

    // Таймер считает количество времени
    calibrate_time+=0.001;
    // Пока датчика нет вот так в ручном режиме увеличиваем
    // current+=0.1;
    Serial.println(current);
  }
  if(current <= 0.06){
    // Останавливаем
    digitalWrite(D8, 0);
    digitalWrite(D9, 0);
    digitalWrite(D10, 0);   
    HC05_SERIAL.write("Calibrate is over   ");

    HC05_SERIAL.write("2. Exit             ");
    if (HC05_SERIAL.available()){
      check_value = 0;
      readValue = HC05_SERIAL.read();
      check_value = atoi (readValue.c_str());
      char ch = static_cast<char>(check_value);
        if(ch == '1'){
          main_menu();
        }
    }
  }
}


void position_main(){
  float set_position;

  set_position = value_position;


  if (position_now != set_position){
    // Раскрутить
    if (position_now - set_position < 0){
      // Пока необходимое время-расстояние меньше нынешнего  потраченого времени
      if((abs(position_now - set_position) * calibrate_time) >= check_time){

        digitalWrite(D8, 0);
        digitalWrite(D9, 1);
        digitalWrite(D10, 1);  

        if(timer_counter > 0) check_time+=(0.001 * 1.3); else check_time+=(0.001 * 2);
        Serial.println(check_time);
        Serial.println("Сработало открутить 1 раз");
      }

      // когда не входит в while выше, то установить позицию сейчас в желаемую позицию
      if(check_time > (abs(position_now - set_position) * calibrate_time)){
        check_time = 0.0;
        position_now = set_position;
      }
    }
    // Закрутить
    if (position_now - set_position > 0){
      // Пока необходимое время-расстояние меньше нынешнего потраченого времени
      if((abs(position_now - set_position) * calibrate_time) >= check_time){

        digitalWrite(D9, 0);
        digitalWrite(D8, 1);
        digitalWrite(D10, 1);      
        
        if(timer_counter > 0) check_time+=0.001; else check_time+=0.001 / 0.1;
        Serial.println(check_time);
        Serial.println("Сработало закрутить 1 раз");
        
      }

      if(check_time > (abs(position_now - set_position) * calibrate_time)){
        check_time = 0.0;
        position_now = set_position;
      }
    }

  }
  if(position_now == set_position){
    // Остановить и установить позицию сейчас в желаемую позицию
    digitalWrite(D8, 0);
    digitalWrite(D9, 0);
    digitalWrite(D10, 0);    

    // HC05_SERIAL.write("Положение устройства в желаемом положении");

    // HC05_SERIAL.write("Для выхода в главное меню нажмите - 1.");
    if (HC05_SERIAL.available()){
      check_value = 0;
      readValue = Serial.readString();
      check_value = atoi (readValue.c_str());
        if(check_value == 1){
          main_menu();
        }
    }    
  }

}