#include <Wire.h> 
#include<LiquidCrystal.h>
#include<EEPROM.h>
#include <DHT.h>
#include<TimerOne.h>

#define DHTPIN A2
#define DHTTYPE DHT11 

#define sw1 3                 //Nút bấm số 1
#define sw2 4                 //Nút bấm số 2
#define sw3 5                 //Nút bấm số 3
#define sw4 6                 //Nút bấm số 4
#define buzzer 7              //Chân điều khiển còi chip
#define solid 8               //Chân điều khiển Solid state bật/tắt máy phun sương
#define wait_solid 9          //Chân chờ điều khiển thiết bị mở rộng
#define t_add 0               //Địa chỉ lưu giá trị nhiệt độ trong EEPROM
#define h_add 1               //Địa chỉ lưu giá trị độ ẩm trong EEPROM


 
/* Địa chỉ của DS1307 */
const byte DS1307 = 0x68;
/* Số byte dữ liệu sẽ đọc từ DS1307 */
const byte NumberOfFields = 7;
 
/* khai báo các biến thời gian */
int second, minute, hour, day, wday, month, year; //giây, phút, giờ, thứ, ngày, tháng, năm
int humi, temp, t, h;                             //Biến lưu nhiệt độ, độ ẩm đo hiện tại và lưu trong EEPROM
unsigned char mode = 0, count = 0, dem, state = 0;                //Biến lưu chế độ hoạt động của hệ thống, đếm thời gian

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal lcd(A1,A0,13,12,11,10);

void setup() {
  // put your setup code here, to run once:
  pinMode(sw1, INPUT);
  pinMode(sw2, INPUT);
  pinMode(sw3, INPUT);
  pinMode(sw4, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(solid, OUTPUT);
  pinMode(wait_solid, OUTPUT);
  lcd.begin(20,4);
  Wire.begin();
  /* cài đặt thời gian cho module */
  //setTime(15, 05, 45, 4, 6, 12, 17); // 15:05:45 T4 06-12-2017
  Serial.begin(9600);
  attachInterrupt(0, setMode, FALLING);  //Ngắt ngoài 0, cài đặt chế độ của hệ thống
  attachInterrupt(1, setState, FALLING); //Ngắt ngoài 1, chọn trạng thái của hệ thống trong từng chế độ
  t = EEPROM.read(t_add);                //Đọc ngưỡng nhiệt độ cảnh báo được lưu trong EEPROM
  delay(250);
  h = EEPROM.read(h_add);                //Đọc ngưỡng độ ẩm cảnh báo được lưu trong EEPROM
  delay(250);
}

void loop() {
  // put your main code here, to run repeatedly:
  //Chế độ điều khiển tự động
  if(mode == 0){
    if(dem == 0){
      lcd.clear();
      dem = 1;  
    }
    if(dem == 1){
      readDS1307();
      humi = dht.readHumidity();
      temp = dht.readTemperature();
      disPlay();
      lcd.setCursor(9, 3);
      lcd.print("Auto mode  ");
      delay(10);
      if(humi < h){
        digitalWrite(solid, HIGH);
        lcd.setCursor(3,3);
        lcd.print("On "); 
        delay(10);   
      }
      if(humi > (h + 20)){
        digitalWrite(solid, LOW);
        lcd.setCursor(3,3);
        lcd.print("Off"); 
        delay(10); 
      }
    }
    delay(200);

    //Kiểm tra trạng thái máy phun sương
    if(!digitalRead(solid)){
        lcd.setCursor(3,3);
        lcd.print("Off"); 
        delay(10);    
    }
    if(digitalRead(solid)){
        lcd.setCursor(3,3);
        lcd.print("On "); 
        delay(10);    
    }
  }

  //Chế độ điều khiển bằng tay
  if(mode == 1){
    readDS1307();
    humi = dht.readHumidity();
    temp = dht.readTemperature();
    disPlay();
    lcd.setCursor(9, 3);
    lcd.print("Manual mode");
    delay(20);
    if(!digitalRead(sw2)){
      delay(50);
      if(!digitalRead(sw2)){
        digitalWrite(solid, HIGH);
        lcd.setCursor(3,3);
        lcd.print("On "); 
        delay(20);   
      }    
    }
    if(!digitalRead(sw3)){
      delay(50);
      if(!digitalRead(sw3)){
        digitalWrite(solid, LOW);
        lcd.setCursor(3,3);
        lcd.print("Off"); 
        delay(20);   
      }    
    }
    delay(200);
  }

  //Chế độ cài đặt giờ bằng tay
  if(mode == 2){
    if(dem == 0){
      lcd.clear();
      dem = 1;
      readDS1307();
    }
    if(dem == 1){
      setupTime();
    }  
  }

  //Chế độ cài đặt nhiệt độ, độ ẩm
  if(mode == 3){
    if(dem == 0){
        lcd.clear();
        dem = 1;
      }
    if(dem == 1){
      setTempHumi();
    }  
  }

  //Chức năng cảnh báo nhiệt độ quá thấp(khi nhiệt độ thấp hơn nhiệt độ cảnh báo được lưu trog EEPROM)
  if(second == 50 && (minute % 2) == 0){
      if(temp < t){
          digitalWrite(buzzer, HIGH);
          delay(500);
          digitalWrite(buzzer, LOW);          
        }
    }
}

//Hàm cài đặt chế độ hoạt động của hệ thống, dùng ngắt ngoài 0
void setMode(){
  mode++;
  dem = 0;
  state = 0;
  if(mode > 3) mode = 0;  
}

//Hàm lựa chọn trạng thái của hệ thống trong từng chế độ hoạt động, dùng ngắt ngoài 1
void setState(){  
  state++;
}

//Hàm hiển thị các thông số
void disPlay(){
  lcd.setCursor(0,0);
  //Hiển thị thứ
  if(wday == 1) lcd.print("CN");
  else {
    lcd.print("T");
    lcd.print(wday);  
  }
  delay(10);
  
  //Hiển thị ngày/tháng/năm
  lcd.setCursor(3,0);
  lcd.print(day);
  delay(10);
  lcd.print("/");
  delay(10);
  lcd.print(month);
  delay(10);
  lcd.print("/");
  delay(10);
  lcd.print(year);
  delay(10);
  
  //Hiển thị giờ:phút:giây
  lcd.setCursor(12,0);
  lcd.print(hour);
  delay(10);
  lcd.print(":");
  delay(10);
  lcd.print(minute);
  delay(10);
  lcd.print(":");
  delay(10);
  lcd.print(second);
  delay(10);
  lcd.print("  ");
  delay(10);

  //Hiển thị nhiệt độ
  lcd.setCursor(0,1);
  lcd.print("Cur ");
  delay(10);
  lcd.print(temp);
  delay(10);
  lcd.print("*C");
  delay(10);
  lcd.setCursor(12,1);
  delay(10);
  lcd.print("Set "); 
  delay(10);
  lcd.print(t);  
  delay(10);
  lcd.print("*C"); 
  delay(10);

  //Hiển thị độ ẩm
  lcd.setCursor(0,2);
  lcd.print("Cur ");
  delay(10);
  lcd.print(humi);
  delay(10);
  lcd.print("%");
  delay(10);
  lcd.setCursor(12,2);
  delay(10);
  lcd.print("Set "); 
  delay(10);
  lcd.print(h);  
  delay(10);
  lcd.print("%");
  delay(10);

  //Hiển thị trạng thái máy phun sương
  lcd.setCursor(0,3);
  lcd.print("PS ");
  delay(10);
}

//Hàm cài đặt thời gian
void setupTime(){
  if(state > 7) state = 0;
  if(state == 0){
      lcd.setCursor(1, 0);
      lcd.print("Set up Timer mode");
      delay(10);
    }
    //Cài đặt giây
  if(state == 1) {
      lcd.setCursor(0,1);
      lcd.print("Second: ");
      lcd.print(second);
      lcd.print(" ");
      delay(10);
      if(!digitalRead(sw2)){
        delay(50);
        if(!digitalRead(sw2)) second++;
        }
      if(!digitalRead(sw3)){
        delay(50);
        if(!digitalRead(sw3)) second--;
        }
      if(second > 59) second = 0;
      if(second <0) second = 59;
      }   

      //Cài đặt phút
   if(state == 2) {
      lcd.setCursor(0,1);
      lcd.print("Minute: ");
      lcd.print(minute);
      lcd.print(" ");
      delay(10);
      if(!digitalRead(sw2)){
        delay(50);
        if(!digitalRead(sw2)) minute++;
        }
      if(!digitalRead(sw3)){
        delay(50);
        if(!digitalRead(sw3)) minute--;
        }
      if(minute > 59) minute = 0;
      if(minute <0) minute = 59;
      }   

      //Cài đặt giờ
    if(state == 3) {
      lcd.setCursor(0,1);
      lcd.print("Hour: ");
      lcd.print(hour);
      lcd.print("   ");
      if(!digitalRead(sw2)){
        delay(50);
        if(!digitalRead(sw2)) hour++;
        }
      if(!digitalRead(sw3)){
        delay(50);
        if(!digitalRead(sw3)) hour--;
        }
      if(hour > 23) hour = 0;
      if(hour <0) hour = 23;
      }   

      //Cài đặt thứ trong tuần
    if(state == 4) {
      lcd.setCursor(0,1);
      lcd.print("Day: ");
      lcd.print(day);
      lcd.print("     ");
      if(!digitalRead(sw2)){
        delay(50);
        if(!digitalRead(sw2)) day++;
        }
      if(!digitalRead(sw3)){
        delay(50);
        if(!digitalRead(sw3)) day--;
        }
      if(day > 7) day = 1;
      if(day <1) day = 7;
      } 

      //Cài đặt ngày
    if(state == 5) {
      lcd.setCursor(0,1);
      lcd.print("Date: ");
      lcd.print(wday);
      lcd.print("     ");
      if(!digitalRead(sw2)){
        delay(50);
        if(!digitalRead(sw2)) wday++;
        }
      if(!digitalRead(sw3)){
        delay(50);
        if(!digitalRead(sw3)) wday--;
        }
      if(wday > 31) wday = 1;
      if(wday <1) wday = 31;
      }  

      //Cài đặt tháng
    if(state == 6) {
      lcd.setCursor(0,1);
      lcd.print("Month: ");
      lcd.print(month);
      lcd.print("   ");
      if(!digitalRead(sw2)){
        delay(50);
        if(!digitalRead(sw2)) month++;
        }
      if(!digitalRead(sw3)){
        delay(50);
        if(!digitalRead(sw3)) month--;
        }
      if(month > 12) month = 1;
      if(month <1) month = 12;
      }    
       //Cài đặt năm
    if(state == 7) {
      lcd.setCursor(0,1);
      lcd.print("Year: ");
      lcd.print(year);
      lcd.print("   ");
      if(!digitalRead(sw2)){
        delay(50);
        if(!digitalRead(sw2)) year++;
        }
      if(!digitalRead(sw3)){
        delay(50);
        if(!digitalRead(sw3)) year--;
        }
      if(year > 99) year = 0;
      if(year <0) year = 99;
    } 

      //Lưu cài đặt và thoát chế độ cài đặt về chế độ Auto                            
    if(!digitalRead(sw4)){
      delay(50);
      if(!digitalRead(sw4)){
        setTime(hour, minute, second, day, wday, month, year); 
        mode = 0; state = 0; dem = 0;
      }
    }                   
}

//Hàm cài đặt nhiệt độ, độ ẩm
void setTempHumi(){
  if(state > 2) state = 0;
  if(state == 0){
    //lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set up Temp and Humi"); 
  } 

   //Cài đặt nhiệt độ cảnh báo
  if(state == 1){
    lcd.setCursor(0,1);
    lcd.print("Set up temp: ");
    lcd.print(t);
    if(!digitalRead(sw2)){
        delay(50);
        if(!digitalRead(sw2)) t++;
        }
    if(!digitalRead(sw3)){
        delay(50);
        if(!digitalRead(sw3)) t--;
        }  
  }

    //Cài đặt độ ẩm cảnh báo
  if(state == 2){
    lcd.setCursor(0,1);
    lcd.print("Set up humi: ");
    lcd.print(h);
    if(!digitalRead(sw2)){
        delay(50);
        if(!digitalRead(sw2)) h++;
        }
      if(!digitalRead(sw3)){
        delay(50);
        if(!digitalRead(sw3)) h--;
        }  
  }
  if(!digitalRead(sw4)){
      delay(50);
      if(!digitalRead(sw4)){
        EEPROM.write(t_add, t);
        delay(50);
        EEPROM.write(h_add, h);
        delay(50); 
        mode = 0; state = 0; dem = 0;
      }
    }     
}

