#include<pitches.h>               //스피커 출력을 위한 라이브러리
#include<LiquidCrystal_I2C.h>     //LCD 출력을 위한 라이브러리
LiquidCrystal_I2C lcd(0x27,20,4);
#include <Wire.h>

//입출력 센서 핀
int temp_pin=A0, humid_pin=A1, light_pin=A2;
int button_pin=16;
int speaker_pin=57;
int rgb_pin[]={6,7,8};

unsigned long time_previous, time_current;  //하루동안 for loop이 돌 수 있도록하는 시간 저장 변수

//스피커 관련
int melody0[]={NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5}; //시작음/도미솔도
int melody1[]={NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4}; //생장음/도레미파
int melody2[]={NOTE_F4, NOTE_F4, NOTE_F4, NOTE_C5}; //개화음/파파파도
int melody3[]={NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C4}; //사망음/도솔미도
int noteDuration=4; //4분 음표

//식물 정보
int day_time=30*1000;                   //하루에 해당하는 시간(초)
int lifetime=5*day_time;                //식물의 수명
int night_len=10*1000;                  //암기
int water[5]={0,1,2,3,4};               //물 준 횟수 기준표
int temp[6] = {0, 5, 10, 25, 30, 35}; //잘 자라는 온도 범위 기준표
int ischange[5]={1,1,1,0,1};            //성장단계에 '변화'가 생기는지에 대한 배열.배열 크기는 수명 일 수

unsigned long check_start, check_end;   //하루동안 '매 초' 실행할 코드를 위한 시간 저장 변수

int water_N;          //하룻동안 물을 준 횟수
boolean check_water;  //흙이 촉촉한 상태인지 건조한 상태인지
float Hscore;         //하루동안 물을 준 횟수에 대한 환산 점수

int temp_check; //하룻동안 온도를 체크한 횟수
float Tscore;   //하루동안 체크한 온도 환산 점수

unsigned long night_start, night_end; //하루동안의 암기측정을 위한 시간변수
int Lflag, Hflag;                     //각각 조도&습도 변화 감지
unsigned long night_time;             //측정한 최대 암기

int day_count=0;      //몇일차인지 표현
int plant_level=0;    //식물의 성장 단계
int speaker_level=-1; //스피커 출력시 시작/개화/생장/사망을 구분하기 위한 변수
int grow_index=0;     //ischange의 인덱스로 쓰임
int health_point=1;   //건강 지수

//여기부터 lcd 문자

byte potL[8] = {     //시작할 때 화분 lcd
        B00000,
        B00000,
        B11111,
        B11111,
        B00111,
        B00111,
        B00111,
        B00111};

byte potR[8] = {
        B00000,
        B00000,
        B11111,
        B11111,
        B11100,
        B11100,
        B11100,
        B11100};

byte seed1[8] = {    //여기부터 씨앗
        B00000,
        B00111,
        B01000,
        B01000,
        B00100,
        B00011,
        B00000,
        B00000};

byte seed2[8] = {
        B00000,
        B11100,
        B00010,
        B00010,
        B00100,
        B11000,
        B00000,
        B00000};      //여기까지 씨앗. 차례대로 왼쪽, 오른쪽.


byte leaf1[8] = {     //leaf1, leaf2는 새싹
        B00000,
        B00000,
        B00000,
        B00000,
        B01110,
        B00111,
        B00001,
        B00001};

byte leaf2[8] = {
        B00000,
        B00000,
        B00000,
        B00000,
        B01110,
        B11100,
        B10000,
        B10000};

byte seed1_1[8] = {       //새싹이 연결된 씨앗
        B00001,
        B00111,
        B01001,
        B01010,
        B00101,
        B00011,
        B00000,
        B00000};

byte seed2_1[8] = {
        B10000,
        B11100,
        B10010,
        B01010,
        B10100,
        B11000,
        B00000,
        B00000};

byte leaf1_1[8] = {   //leaf1_1은 leaf 에서 조금 더 성장한 leaf
        B00001,
        B00001,
        B01101,
        B11111,
        B00111,
        B00011,
        B00001,
        B00001};

byte leaf2_1[8] = {
        B10000,
        B10000,
        B10110,
        B11111,
        B11100,
        B11000,
        B10000,
        B10000};

byte headL[8] = {       //꽃 피기 전 봉우리(왼(L), 오(R))
        B00000,
        B00000,
        B00000,
        B00000,
        B00001,
        B00011,
        B00011,
        B00001};

byte headR[8] = {
        B00000,
        B00000,
        B00000,
        B00000,
        B10000,
        B11000,
        B11000,
        B10000};

byte leaf1_2[8] = {       //조금 더 더 성장한 leaf
        B10001,
        B01001,
        B01101,
        B01111,
        B01111,
        B00111,
        B00001,
        B00001};

byte leaf2_2[8] = {
        B10001,
        B10010,
        B10110,
        B11110,
        B11110,
        B11100,
        B10000,
        B10000};

byte headL_1[8] = {       //개화 성공한 꽃 왼(L),오(R)
        B01101,
        B00111,
        B11011,
        B01110,
        B01100,
        B11110,
        B00111,
        B01101};

byte headR_1[8] = {
        B10110,
        B11100,
        B11011,
        B01110,
        B00110,
        B01111,
        B11100,
        B10110};

byte headL_2[8] = {      //개화 실패한 꽃 왼(L),오(R)
        B00000,
        B00000,
        B00001,
        B00011,
        B00011,
        B00111,
        B00011,
        B00001};

byte headR_2[8] = {
        B00000,
        B00000,
        B10000,
        B11000,
        B11000,
        B11100,
        B11000,
        B10000};

byte skullL[8] = {     //식물 사망 시 띄우는 해골. 왼 위,오른 위,왼 아래,오른 아래
        B00000,
        B00111,
        B01000,
        B01110,
        B10110,
        B10110,
        B01001,
        B00111};

byte skullR[8] = {
        B00000,
        B11100,
        B00010,
        B01110,
        B01101,
        B01101,
        B10010,
        B11100};

byte skullLD[8] = {
        B11001,
        B11100,
        B00010,
        B00001,
        B00001,
        B00010,
        B11100,
        B11000};

byte skullRD[8] = {
        B10011,
        B00111,
        B01000,
        B10000,
        B10000,
        B01000,
        B00111,
        B00011};

//사운드 재생 함수
void play_sound(int *x){    //사운드 배열을 포인터 값으로 받음
  if(x[1]==NOTE_E4) Serial.println("**시작음**");      //네가지 사운드 배열의 1번 인덱스에 해당하는 값이 모두 달라 네 사운드를 구분하기위한 인자로 사용함.
  else if(x[1]==NOTE_D4) Serial.println("**생장음**");
  else if(x[1]==NOTE_F4) Serial.println("**개화음**");
  else if(x[1]==NOTE_G4) Serial.println("**사망음**");
  
  for(int thisNote=0;thisNote<4;thisNote++){
    int noteLength=1000/noteDuration;
    tone(speaker_pin,x[thisNote],noteLength);
    delay(noteLength);
    noTone(speaker_pin);
  }
}

void change_plant_level(int x){ //식물의 단계 변화(LCD변환 및 사운드 재생)
  //x가 5일 때 0인 상태로 변환시키는 것까지
  
  int i=0;    // write 코드 줄이기 용 인덱스 변수인데 하나만 사용했다.(이후 수정 가능성 높음)
  switch(x){
    case 0:
      //lcd.setCursor(0,0);
      lcd.init();
      lcd.print("Push Button to Start");  //시작 전 LCD에 띄울 문구

      lcd.createChar(0, potL);
      lcd.createChar(1, potR);   
      lcd.createChar(2, headL_1);
      lcd.createChar(3, headR_1);
        
      lcd.setCursor(9,2);
      lcd.write(byte(2));
      lcd.write(byte(3));
    
      lcd.setCursor(9,3);
      lcd.write(byte(0));
      lcd.write(byte(1));
      break;
      
    case 1: //생장 1단계(씨앗) 출력
      lcd.createChar(0, seed1);
      lcd.createChar(1, seed2);
      //lcd.begin(16, 2);
      lcd.init();
      lcd.backlight();
      
      lcd.setCursor(0,0);     //좌측 상단에 몇일차인지 출력
      lcd.print("Day"+String(day_count+1));
      lcd.setCursor(0,1);     //좌측 중앙에 생장 몇단계인지 출력
      lcd.print("Lv."+String(plant_level));    
  
      lcd.setCursor(9,3);
      lcd.write(byte(0)); 
      lcd.write(byte(1)); 
      break;

    case 2: //생장 2단계(새싹) 출력
      lcd.createChar(0, leaf1);
      lcd.createChar(1, leaf2);
      lcd.createChar(2, seed1_1);
      lcd.createChar(3, seed2_1);

      //lcd.begin(16, 2);
      lcd.init();
      lcd.backlight();
      
      lcd.setCursor(0,0);     //좌측 상단에 몇일차인지 출력
      lcd.print("Day"+String(day_count+1));  
      lcd.setCursor(0,1);     //좌측 중앙에 생장 몇단계인지 출력
      lcd.print("Lv."+String(plant_level));    
      
      lcd.setCursor(9,2);
      lcd.write(byte(0));
      lcd.write(byte(1));
  
      lcd.setCursor(9,3);
      lcd.write(byte(2));
      lcd.write(byte(3));
      break;
      
    case 3: //생장 3단계(어린줄기) 출력
      lcd.createChar(0, leaf1_1);
      lcd.createChar(1, leaf2_1);   
      lcd.createChar(2, headL);
      lcd.createChar(3, headR);
      //lcd.begin(16, 2);
      lcd.init();
      lcd.backlight();

      lcd.setCursor(0,0);     //좌측 상단에 몇일차인지 출력
      lcd.print("Day"+String(day_count+1));   
      lcd.setCursor(0,1);     //좌측 중앙에 생장 몇단계인지 출력
      lcd.print("Lv."+String(plant_level));   

      lcd.setCursor(9,2);
      lcd.write(byte(2));
      lcd.write(byte(3));
  
      lcd.setCursor(9,3);
      lcd.write(byte(0));
      lcd.write(byte(1));    
      break;
      
    case 4: //생장 4단계(개화or비개화) 출력
      if(night_time>night_len){   //개화(장일 식물의 경우 부등호가 반대)
        lcd.createChar(0, leaf1_2);
        lcd.createChar(1, leaf2_2);   
        lcd.createChar(2, headL_1);
        lcd.createChar(3, headR_1);
        //lcd.begin(16, 2);
        lcd.init();
        lcd.backlight(); 

        lcd.setCursor(0,0);     //좌측 상단에 몇일차인지 출력
        lcd.print("Day"+String(day_count+1));  
        lcd.setCursor(0,1);     //좌측 중앙에 생장 몇단계인지 출력
        lcd.print("Lv."+String(plant_level));  
        lcd.setCursor(9,0);     //우측 상단에 개화문구 출력
        lcd.print("*Flowering*");
    
        lcd.setCursor(9,2);
        lcd.write(byte(2));
        lcd.write(byte(3));
    
        lcd.setCursor(9,3);
        lcd.write(byte(0));
        lcd.write(byte(1));
        speaker_level=2;        //개화 사운드 재생 조건(개화가 생장보다 우선순위가 높음!)
      }
      else{                     //비개화
        lcd.createChar(0, leaf1_2);
        lcd.createChar(1, leaf2_2);   
        lcd.createChar(2, headL_2);
        lcd.createChar(3, headR_2);
        //lcd.begin(16, 2);
        lcd.init();
        lcd.backlight();     

        lcd.setCursor(0,0);     //좌측 상단에 몇일차인지 출력
        lcd.print("Day"+String(day_count+1));  
        lcd.setCursor(0,1);     //좌측 중앙에 생장 몇단계인지 출력
        lcd.print("Lv."+String(plant_level));    
    
        lcd.setCursor(9,2);
        lcd.write(byte(2));
        lcd.write(byte(3));
    
        lcd.setCursor(9,3);
        lcd.write(byte(0));
        lcd.write(byte(1));
      }
      break;
      
    default:  //생장 5단계(사망) 출력
      lcd.createChar(0, skullL);
      lcd.createChar(1, skullR);   
      lcd.createChar(2, skullLD);
      lcd.createChar(3, skullRD);
      //lcd.begin(16, 2);
      lcd.init();
      lcd.backlight(); 

      lcd.setCursor(1,0);     //중앙 상단에 사망 문구 출력
      lcd.print("The Plant is Dead");
      lcd.setCursor(4,3);     //좌측 상단에 사망 문구 출력
      lcd.print("Push Button");
  
      lcd.setCursor(9,1);
      lcd.write(byte(0));
      lcd.write(byte(1));
  
      lcd.setCursor(9,2);
      lcd.write(byte(2));
      lcd.write(byte(3));
 
      //프로그램 초기 상태로 변수 초기화
      day_count=0;    
      plant_level=0;
      grow_index=0;
      health_point=1;
      
      play_sound(melody3);    //사망 사운드 재생
      digitalWrite(rgb_pin[0],HIGH);  //RGB LED 끄기
      digitalWrite(rgb_pin[1],HIGH);
      digitalWrite(rgb_pin[2],HIGH);
      
      Serial.println("처음 상태로 돌아가려면 버튼을 누르시오.");
      while(1){
        if(digitalRead(button_pin)){  //버튼이 눌릴 때까지 머무르게 함.
          Serial.println("(Pushed)");
          delay(500);   //디바운싱
          break;
        }
      }
      speaker_level=-1; //switch문을 나왔을 때 사운드 재생이 안되도록
      change_plant_level(0);  //처음 시작 전 상태로 돌아감
      break;
  }
  if(speaker_level==0) play_sound(melody0);     //LCD 출력 변화 후 speaker level에 따라 소리 재생
  else if(speaker_level==1) play_sound(melody1);
  else if(speaker_level==2) play_sound(melody2);
  
  
  
}

void rgb_on(int x){   //x(건강지수)에 따라 적절한 RGB LED 출력
  if(x<=0){   //건강지수가 0 이하인 경우 Red 색상 출력
    digitalWrite(rgb_pin[0],HIGH); 
    digitalWrite(rgb_pin[1],HIGH);
    digitalWrite(rgb_pin[2],LOW);
  }else if(x==1){ //건강지수가 1일 경우 Yellow 색상 출력
    digitalWrite(rgb_pin[0],HIGH);
    digitalWrite(rgb_pin[1],LOW);
    digitalWrite(rgb_pin[2],LOW);
  }else{          //건강지수가 2 이상인 경우 Green 색상 출력
    digitalWrite(rgb_pin[0],HIGH);
    digitalWrite(rgb_pin[1],LOW);
    digitalWrite(rgb_pin[2],HIGH);
  }
}

void setup() {
  Serial.begin(9600);
  //lcd.begin(20,4);
  //lcd.clear();
  lcd.init();
  lcd.backlight();

  for(int i=0;i<3;i++){   //RGB LED 셋팅
    pinMode(rgb_pin[i],OUTPUT);
  }
  rgb_on(health_point);
  
  change_plant_level(0);
}


void loop() {
  
  if(day_count==0){
    digitalWrite(rgb_pin[0],HIGH);    //RGB LED 끄기
    digitalWrite(rgb_pin[1],HIGH);
    digitalWrite(rgb_pin[2],HIGH);
    Serial.println("식물 키우기를 시작하려면 버튼을 누르시오.");
    while(1){   //버튼 입력이 들어오면 무한루프를 빠져나온다.(day_count가 0일때만 입력을 받도록 해야함)
      if(digitalRead(button_pin)){
        Serial.println("(Pushed)");
        speaker_level=0;
        plant_level=1;
        break;
      }
    }
  }
  rgb_on(health_point);
  change_plant_level(plant_level); 
  
  day_count++;
  time_previous=millis();

  speaker_level=-1;

  Tscore=0;     //온도/습도 점수 초기화
  Hscore=0;
  night_time=0;
  Lflag=0;
  Hflag=0;
  water_N=0;
  temp_check=0;
  check_start=millis();


  Serial.println("-------------------Start "+String(day_count)+"th day-------------------");
  Serial.println("현재 성장단계"+String(plant_level));
  
  for(time_current=millis();time_current-time_previous<=day_time;time_current=millis()){ //하루동안
    //조도 센서
    if(Lflag==0&&analogRead(light_pin)>700){    //500은 임의의 기준값
      night_start=millis();
      Lflag=1;
      Serial.println("어두워짐");
    }
    
    if(Lflag==1&&analogRead(light_pin)<=700){
      night_end=millis();
      if(night_end-night_start>night_time){
        night_time=night_end-night_start;
      }
      Lflag=0;
      Serial.println("밝아짐");
    }

    check_end=millis();
    if(check_end - check_start >= 1000){
      check_start = check_end;
      //습도센서
      if(Hflag==0&&analogRead(humid_pin)<=500){    //500은 임의의 기준값
        water_N++;
        Hflag=1;
        Serial.println("물 줌");
      }
      if(Hflag==1&&analogRead(humid_pin)>500){
        Hflag=0;
      }
      if(water_N>5){    //물을 과하게 준 경우 사망
        health_point = -5;
        break;
      }
      
      //온도센서
      temp_check++;
      int reading=analogRead(temp_pin);
      float temp0=reading*5.0/1024.0*100; //매 초 측정하는 온도
      Serial.println("temp : "+String(temp0)+"\thumid : "+String(analogRead(humid_pin))+"\tCDS : "+String(analogRead(light_pin)));
      
      if(temp0<0 || 80<temp0){     // 온도가 너무 낮거나 너무 높은 경우 사망
        Serial.println("온도가 너무 낮거나 높습니다");
        health_point = -5;
        break;
      }
      else if(temp0<temp[0]) Tscore -= 1;    // 각 온도 범위에서 특정 온도 미만/초과는 -1점. 그 외의 각 범위 사이는 0,2,4점을 매김.
      else if(temp[0]<=temp0 && temp0<temp[1]) Tscore += 0;
      else if(temp[1]<=temp0 && temp0<temp[2]) Tscore += 2;
      else if(temp[2]<=temp0 && temp0<temp[3]) Tscore += 4;
      else if(temp[3]<=temp0 && temp0<temp[4]) Tscore += 2;
      else if(temp[4]<=temp0 && temp0<temp[5]) Tscore += 0;
      else Tscore -= 1;
        
    }
    
  }
  if(Lflag==1){ //만약 하루가 끝날때까지 계속 어두운 상태라면
      night_end=millis();
      if(night_end-night_start>night_time){
        night_time=night_end-night_start;
      }
      Lflag=0;
    }  
  Serial.println("-------------------End "+String(day_count)+"th day-------------------");
  
  //온도&습도점수 계산
  Tscore=Tscore/temp_check;
  Serial.print("Tscore=");
  Serial.print(Tscore);
  
  if(water[0]<=water_N && water_N<water[1]) Hscore = 2;       //하룻동안 물 준 횟수에 따라 습도점수 배점
  else if(water[1]<=water_N && water_N<water[2]) Hscore = 3;
  else if(water[2]<=water_N && water_N<water[3]) Hscore = 2;
  else if(water[3]<=water_N && water_N<water[4]) Hscore = 1;
  else Hscore = 0;
  Serial.print("\tHscore=");
  Serial.println(Hscore);  

  Serial.print("Total Score:");
  Serial.println(Tscore+Hscore);
  
  //건강 지수 변화
  if(Tscore+Hscore<=3) health_point--;
  else if(5<Tscore+Hscore) health_point++;

  Serial.print("변화 후 건강지수:");
  Serial.println(health_point);

  Serial.print("오늘의 최대 암기:");
  Serial.println(String(night_time/1000.0)+"초");
  
  if(day_count==5||health_point<0){  //식물의 사망 조건
    plant_level=5; //5단계(사망) 출력
    change_plant_level(plant_level);
 
  }else{    
    if(health_point!=0){      //식물의 생장 조건
      if(ischange[grow_index]){   //식물의 생장 체계가 바뀔 것을 대비(ex:1단계에서 2단계로 바뀌는데 3일간 최적조건에 있어야한다면, 건강지수가 0일 때마다 하루씩 생장하는 일정이 밀리게 된다.) 
        plant_level++;
        speaker_level=1;      //생장 사운드 재생 조건
      }else speaker_level=-1; //아무 사운드도 재생하지 X
      grow_index++;
    }
  }
  
     
  
}
