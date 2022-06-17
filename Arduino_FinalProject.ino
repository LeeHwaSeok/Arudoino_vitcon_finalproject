/*
  이름 : 이동준, 박준우
  날짜 : 220617
  내용 : 아두이노 final project
 1. 환경 모니터링기능
 2. FAN, PUMP, LED 수동 조작
   : 동작 시간만 설정
 3. FAN, PUMP, LED 자동 조작
   : 제어 장치에 따른 옵션 부여
 4. 장치 상태 표시
   : MODLINK OLED 표기 내용
   4-1 
  : 센서 측정 정보
  : FAN, PUMP, LED ON/OFF
  : MODE 표기
*/
/*  라이브 선언 및 Port(pin) 번호 설정   */ 
#include <VitconBrokerComm.h> 
using namespace vitcon; 

//DHT (온도/습도) 
#include "DHT.h"  // DHT 사용 
#define DHTPIN A1    // 온/습도 - A1 pin
#define DHTTYPE DHT22   // DHT22 버전을 사용하겠다. 
DHT dht(DHTPIN, DHTTYPE); // 핀 = A1 , 모델 = DHT22

//SOIL-LINK (토양 습도)
#define SOILHUMI A6  // 토양 습도 - A6 pin

//PUMP (펌프)
#define PUMP 16

//LED (조명)
#define LAMP 17

//PWM (펜)
#include <SoftPWM.h> // DC 모터 사용
SOFTPWM_DEFINE_CHANNEL(A3); // DC 모터 - A3 pin

//OLED 
#include <U8g2lib.h> // OLED 사용
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
/*---*/

/* 아두이노 변수 */
float temp = 0;
float humi = 0;
int soil = 0;
unsigned long fan_goal = 0;
unsigned long pump_goal = 0;
unsigned long lamp_goal = 0;
unsigned long mils = (millis() / 1000);
int fm = 0;
int pm = 0;
int lm = 0;

String flag_f =""; // Pan On/Off
String flag_l =""; // Lamp On/Off 
String flag_p =""; // Pump On/Off


/* 변수 선언 a0 ~ a11 */
int fan_track = 0;
int pump_track = 0;
int led_track = 0 ;
bool fan; // 펜
bool pump; // 펌프
bool lamp; // 조명

/* 변수 선언 a12 ~ a22 */
bool a_fan;   // auto fan
bool btn_between; // 구동 온도 사이 풍량 증가
bool btn_over; // 최고 온도 초과 풍량 증가
int fan_min = 0;  // 구동온도 min
int fan_max = 0;  // 구동온도 max
int between = 0;  // 현재 구동 온도 사이 풍량 
int over = 0;  // 현재 최고 온도 초과 풍량 

/* 변수 선언 a23 ~ aa33*/
bool a_pump;
bool pump_sw;
bool btn_pump_cnt;
int pump_cnt = 0;
int pump_begin = 0;
int pump_end = 0;

/* 변수 선언 aa34 ~ aa43*/
bool a_led;
bool bool_hour;   //시간 증가
bool bool_minute; //분 증가
bool bool_second; //초 증가
bool begin_time; //시작 여부 

int hour = 0;  //현재 시간
int minute = 0;   //현재 분
int sec = 0;   //현재 초
int ms = 1000; //수식용 ms
//------------------

uint32_t TimeSum = 0; 
uint32_t TimeCompare; 
uint32_t TimePushDelay = 0; 
uint32_t TimerStartTime = 0; 
uint32_t tc;
/*---*/

/* 함수 선언 a0 ~ a11*/
void def_fan_track(int32_t  val)  { fan_track = (int)val;}
void def_pump_track(int32_t  val) { pump_track = (int)val;}
void def_led_track(int32_t  val)  { led_track = (int)val;}

void def_fan(bool val) {   
  fan  = val;   
  delay(500);
  
  if(fan == true){
    fm = mils + fan_track;
    delay(500);
  }
  
}

void def_pump(bool val)   {   
  pump  = val; 
  delay(500);  
  if(pump == true){
    pm = mils + pump_track;
    delay(500);
  }
  

}

void def_lamp(bool val)   {   
  lamp  = val; 
  delay(500);  
  if(lamp == true){
    lm = mils + led_track;
    delay(500);
  }
  
}

/* 함수 선언 a12 ~ a22*/
void def_afan(bool val)  { a_fan  = val; }
void def_fan_low(int32_t val)  { fan_min = (int)val;}
void def_fan_high(int32_t val) { fan_max = (int)val;}
void def_between(bool val)  { btn_between = val;}
void def_over(bool val)  { btn_over = val;}
void def_fan_rst (bool val){
  if(val== true){
 fan_min = 0;
 fan_max = 0;
 between = 0;
 over = 0;
  }
}

/* 함수 선언 a23 ~ aa33*/
void def_apump (bool val) {a_pump = val;}
void def_pump_begin(int32_t val) {pump_begin = (int)val;}
void def_pump_end(int32_t val)   {pump_end = (int)val;}
void def_pump_sw (bool val) {pump_sw = val;}
void def_pump_cnt(bool val) {btn_pump_cnt = val;}

void def_pump_rst(bool val) {
  if(val==true){
 pump_cnt = 0;
 }
}

/* 함수 선언 aa34 ~ aa43*/

void def_aLED(bool val) {a_led = val;}
void def_hour(bool val) { bool_hour = val; }
void def_minute(bool val) { bool_minute = val; }
void def_sec(bool val) { bool_second = val; }
void def_time_Reset(bool val) { 
  if (val == true) { 
 hour = 0; 
 minute = 0; 
 sec = 0;
  } 
} 
void def_time_set(bool val){begin_time = val;}

/*---*/

/* Vitcom 위젯 변수명 선언*/

// Exel 참고하세요
//온도 습도 표시
IOTItemFlo aa0; //숫자/문자열 데이터 출력
IOTItemFlo aa1; //숫자/문자열 데이터 출력 
IOTItemInt aa2; //숫자/문자열 데이터 출력

//매뉴얼 팬, 펌프, led
IOTItemBin aa3; //팬 트렉바 데이터 20sec -> fan true -> 20sec -cnt
IOTItemInt aa4(def_fan_track); // track _Bar  값
IOTItemBin aa5(def_fan);  //true false
IOTItemInt aa6; //펌프 트렉바 데이터 출력
IOTItemInt aa7(def_pump_track);
IOTItemBin aa8(def_pump);
IOTItemInt aa9; //led 트렉바 데이터 출력
IOTItemInt aa10(def_led_track);
IOTItemBin aa11(def_lamp);

//자동 팬 
IOTItemBin aa12;
IOTItemBin aa13(def_afan); //스위치 
IOTItemInt aa14;
IOTItemInt aa15(def_fan_low);
IOTItemInt aa16;
IOTItemInt aa17(def_fan_high);
IOTItemInt aa18;//사이 온도 표시
IOTItemBin aa19(def_between);
IOTItemInt aa20;//초과 온도 표시
IOTItemBin aa21(def_over);
IOTItemBin aa22(def_fan_rst);

//자동 펌프
IOTItemBin aa23;
IOTItemBin aa24(def_apump); //스위치
IOTItemInt aa25;
IOTItemInt aa26(def_pump_begin);
IOTItemInt aa27;
IOTItemInt aa28(def_pump_end);
IOTItemInt aa29; //cnt 출력
IOTItemBin aa30(def_pump_cnt);
IOTItemBin aa31(def_pump_rst);
IOTItemBin aa32;
IOTItemBin aa33(def_pump_sw);

//자동 led
IOTItemBin aa34;
IOTItemBin aa35(def_aLED); //스위치
IOTItemBin aa36(def_hour);
IOTItemInt aa37; //시간 출력
IOTItemBin aa38(def_minute);
IOTItemInt aa39; //분 출력
IOTItemBin aa40(def_sec);
IOTItemInt aa41; //초 출력
IOTItemBin aa42(def_time_set);
IOTItemBin aa43(def_time_Reset);

//Vitcon LED 점등 여부
IOTItemBin aa44; //팬 led
IOTItemBin aa45; //펌프 led
IOTItemBin aa46; //led led

/* --- */

/* vitcom index를 아두이노에서 제어 가능하게 변경  */ 
#define ITEM_COUNT 47
IOTItem *items[ITEM_COUNT] = { 
  &aa0,&aa1,&aa2,&aa3,&aa4,&aa5,&aa6,&aa7,&aa8,&aa9,
  &aa10,&aa11,&aa12,&aa13,&aa14,&aa15,&aa16,&aa17,&aa18,&aa19,
  &aa20,&aa21,&aa22,&aa23,&aa24,&aa25,&aa26,&aa27,&aa28,&aa29,
  &aa30,&aa31,&aa32,&aa33,&aa34,&aa35,&aa36,&aa37,&aa38,&aa39,
  &aa40,&aa41,&aa42,&aa43,&aa44,&aa45,&aa46,
  }; 

const char device_id[] = "736c176e59c2b55481f9d7d633dafe78"; // Change device_id to yours 
BrokerComm comm(&Serial, device_id, items, ITEM_COUNT); 



void _Begin(){
  Serial.begin(250000); 
  comm.SetInterval(200); 
  SoftPWM.begin(490);
  dht.begin();
  u8g2.begin();
  pinMode(PUMP,OUTPUT);
  pinMode(SOILHUMI,INPUT);
  pinMode(LAMP,OUTPUT);
}


void setup() {
  _Begin();
}

void loop() {
  mils = (millis() / 1000);
  tc = (millis()/1000);
  
  //팬 bar -> sec
  if(fm < tc){ fan = false;  }
  else if (fm > tc){  fan = true; }
  
  //펌프 bar -> sec
  if(pm < tc){ pump = false;  }
  else if (pm > tc){ pump = true; }
  
  //led bar -> sec
  if(lm < tc){ lamp = false;  }
  else if (lm > tc){ lamp = true; } 

  //auto fan
  between_Fan();
  Auto_fan(a_fan);
  if(a_fan){
    fan = false;   
     if( temp > fan_min || temp < fan_max){ 
       SoftPWM.set(between);
     }
     else if (temp > fan_max){
       SoftPWM.set(over);
     }
    else{
    SoftPWM.set(0);
    }
  }

  if(a_pump){
    pump = false;   
     if( humi > pump_begin || humi < pump_end){ 
        digitalWrite(PUMP,HIGH);
        flag_p ="O"; 

        de_pump_sw();
     }
    else{
        digitalWrite(PUMP,LOW);
        flag_p ="X"; 
    }
  }
  
  if(a_led){
     InvervalSet(a_led,begin_time);   
  }
  
  Serial.print("fan : ");
  Serial.println(fan);
  Serial.print("pump : ");
  Serial.println(pump);
  Serial.print("lamp : ");
  Serial.println(lamp);
  Serial.print("mils : ");
  Serial.println(fm);
  Serial.print("fan_goal : ");
  Serial.println(fan_track);
  Serial.print("millis() : ");
  Serial.println(tc);
  humi = dht.readHumidity();
  temp = dht.readTemperature();
  soil = map(analogRead(SOILHUMI),0,1023,100,0);  

  pump_cnt += btn_pump_cnt;
  
  Serial.print("between : ");
  Serial.println(between);
  Serial.print("over : ");
  Serial.println(over);
  Serial.print("hour : ");
  Serial.println(hour);
  Serial.print("minute : ");
  Serial.println(minute);
  Serial.print("sec : ");
  Serial.println(sec);
  Serial.print("pump() : ");
  Serial.println(pump);
  Serial.print("lamp() : ");
  Serial.println(lamp);

  
  manual_fan(fan);
  manual_pump(pump);
  manual_led(lamp);

  datasend();
  OLEDdraw();
  comm.Run();

}

void de_pump_sw(){
  if (!a_pump){TimerStartTime = millis();}
  else if (a_pump) { 
    TimeCompare = (millis() - TimerStartTime) / pump_cnt * 1000; 
    if (TimeCompare % 2 == 0) { digitalWrite(PUMP, HIGH); }
    else if (TimeCompare % 2 == 1) { digitalWrite(PUMP, LOW); } 
  }
}

void manual_fan(bool val){
  if (val){
   SoftPWM.set(70);
   aa44.Set(val);
   flag_f = "O";
  }
  else {
   SoftPWM.set(0);
   aa44.Set(val);
   flag_f = "X";
  }
}

void manual_pump(bool val){
  if (val){
   digitalWrite(PUMP,HIGH);
   aa45.Set(val);
   flag_p ="O"; 
  }
  else {
   digitalWrite(PUMP,LOW);
   aa45.Set(val);
   flag_p = "X";
  }
}

void manual_led(bool val){
  if (val){
   digitalWrite(LAMP,HIGH);
   aa46.Set(val);
   flag_l = "0";
  }
  else {
   digitalWrite(LAMP,LOW);
   aa46.Set(val);
   flag_l = "X";
  }
}

/* 풍량 초기화 */
void Auto_fan(bool a_fan){
  if (a_fan){
  between = 0;
  over = 0;
  }
}

/* 풍량 파워 */
void between_Fan() {
  if (millis() > TimePushDelay + 500) { 
   between += btn_between; 
   over += btn_over; 
   TimePushDelay = millis();
  }
}
 
/* 데이터 전송 */
void datasend(){
  aa0.Set(temp);
  aa1.Set(humi);
  aa2.Set(soil);

  aa29.Set(pump_cnt);
  
  aa18.Set(between);
  aa20.Set(over);
  aa37.Set(hour);
  aa39.Set(minute);
  aa41.Set(sec);
  aa45.Set(pump);
  aa46.Set(lamp);
  }

/* 인터벌 타임 */
void InvervalSet(bool lamp_timer, bool begin_ ){

  lamp = false;
  
  if (millis() > TimePushDelay + 300) {  // 조작 시간 0.3초안에 버튼누르면 적용 됨
    hour += bool_hour; 
    minute += bool_minute;
    sec += bool_second;
      
    if (hour >= 24) {hour = 0; }
    if (minute >= 60) {minute = 0;}
    if (sec >= 60) {sec = 0;}
  
     TimePushDelay = millis();      
 }
 
  //램프가 off여야만 조작가능
  if (!lamp_timer){
    TimeSum = (uint32_t)sec*ms + (minute * 60 * ms) + (hour * 60 * 60 * ms) ; // sec + min + hour           
    TimerStartTime = millis(); 
  }

  //on이면 카운트에 맞춰서 다음 실행
  else if (lamp_timer && begin_) { 
   TimeCompare = (millis() - TimerStartTime) / TimeSum; 
   lamp_timeCompare();
  }
}

/* 반짝반짝 LED */
void lamp_timeCompare() {
  if (TimeCompare % 2 == 0) { digitalWrite(LAMP, HIGH); }
  else if (TimeCompare % 2 == 1) { digitalWrite(LAMP, LOW); } 
}


//OLED 함수선언
void OLEDdraw(){
  //매 출력 시작할 때 초기화
  u8g2.clearBuffer();
  
  //폰트 지정
  u8g2.setFont(u8g2_font_ncenB08_te);

  //온도
  u8g2.drawStr(1, 9, "Temp");
  u8g2.setCursor(80,9);
  u8g2.print(temp);
  u8g2.drawStr(114,9,"\xb0");
  u8g2.drawStr(119,9,"C");

  //습도
  u8g2.drawStr(1,20,"Humidity");
  u8g2.setCursor(80,20);
  u8g2.print(humi);
  u8g2.drawStr(116,20,"%");

  //토양 습도 체크
  u8g2.drawStr(1,31,"Soil Humi");
  u8g2.setCursor(95,31);
  u8g2.print(soil);
  u8g2.drawStr(116,31,"%");

  //Fan 상태 출력
  u8g2.drawStr(1,42,"FAN");
  u8g2.setCursor(55,42); 
  if (flag_f=="X"){
 u8g2.print("OFF");
  }
  else if (flag_f=="O"){
 u8g2.print("ON");
  }
  u8g2.drawStr(80,42,"/");
  if (a_fan){
 u8g2.drawStr(90,42,"AUTO");
  }
  else{
 u8g2.drawStr(90,42,"MANU");
  }
  
  //PUMP 상태 출력
  u8g2.drawStr(1,53,"PUMP");
  u8g2.setCursor(55,53);
  if (flag_p=="X"){
 u8g2.print("OFF");
  }
  else if (flag_p=="O"){
 u8g2.print("ON");
  }
  u8g2.drawStr(80,53,"/");
  if (a_pump){
 u8g2.drawStr(90,53,"AUTO");
  }
  else{
 u8g2.drawStr(90,53,"MANU");
  }

  //LED 상태 출력
  u8g2.drawStr(1,64,"LED");
  u8g2.setCursor(55,64);
  if (flag_l=="X"){
 u8g2.print("OFF");
  }
  else if (flag_l=="O"){
 u8g2.print("ON");
  }
  u8g2.drawStr(80,64,"/");
  if (a_led){
 u8g2.drawStr(90,64,"AUTO");
  }
  else{
 u8g2.drawStr(90,64,"MANU");
  }
  
  //버퍼 전송
  u8g2.sendBuffer();
}
