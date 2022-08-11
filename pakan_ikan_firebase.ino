//firebaserido@gmail.com
// pass:firebaserido12
//https://console.firebase.google.com/u/2/project/akuarium-41f66/database/akuarium-41f66-default-rtdb/data/~2F
// library servo
#include <Servo.h>
// object servo
Servo myservo;
// sudut servo saat buka dan nutup
int tutup = 90;
int buka = 150;
// library esp8266
#include <ESP8266WiFi.h>
// library firebase
#include "FirebaseESP8266.h"
// konfigurasi wifi dan firebase
#define WIFI_SSID "Al-Barkah"
#define WIFI_PASSWORD "jangankerok99"
#define FIREBASE_HOST "https://akuarium-41f66-default-rtdb.asia-southeast1.firebasedatabase.app/" 
#define FIREBASE_AUTH "AjTNo7YwLtDCh7yTSBrVe4G4rXHo0oCRxcgAjRx9"
// object firebase
FirebaseData firebaseData;

#include <WiFiUdp.h>
#include <NTPClient.h>
const long utcOffsetInSeconds = 25200;
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org", utcOffsetInSeconds);
//Week Days
String weekDays[7]={"minggu", "senin", "selasa", "rabu", "kamis", "jumat", "sabtu"};
String all_time,hari;
int jam,menit,detik;

// baca data waktu dr ntp
void baca_ntp(){
  // update waktu
  timeClient.update();
  // baca epochtime
  unsigned long epochTime = timeClient.getEpochTime();
  // baca format waktu
  String formattedTime = timeClient.getFormattedTime();
  // baca waktu per bagian
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();
  // salin ke variabel
  jam = currentHour;
  menit = currentMinute;
  detik = currentSecond;
  hari = weekDays[timeClient.getDay()];

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  // ubah ke tanggal bulan tahun
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;
  
  String currentDate =  formattedTime +" "+ hari + " " + String(monthDay)+ "/" + String(currentMonth) + "/" + String(currentYear);
  all_time = currentDate;
  // tampilka ke serial monitor
  Serial.print(" time: ");
  Serial.print(all_time);
  Serial.println();
}
// konfigurasi input output
#define pinbuzzer    D5
#define pinservo     D0
#define pinfloat     A0
#define pinir        D1
#define pinsuhu      D2
#define pinrelay1    D6
#define pinrelay2    D7
// variable pengolah program utama
unsigned long previousMillis = 0;       
const long interval = 1000;
int counter=0;
String text;
String harikuras;
int jamkuras,jampakan1,jampakan2,kurasmanual,pakanmanual,pakantersedia,delaykatup,notifikasi;
float suhu;
// library onewire untuk sensor suhu ds18b20
#include <OneWire.h>
// sensor suhu ds18b20
#include <DallasTemperature.h>
// konfigurasi sensor suhu ds18b20
OneWire configsensor(pinsuhu);
DallasTemperature sensor(&configsensor);
// update firebase
void get_firebase(){
  // baca data 
  if (Firebase.getString(firebaseData, "/harikuras")) {
    Serial.print(" harikuras:");
    harikuras = firebaseData.stringData();
    Serial.println(harikuras);
  } else {
    Serial.println(firebaseData.errorReason());
  } 
  
  if (Firebase.getInt(firebaseData, "/jamkuras")) {
    Serial.print(" jamkuras:");
    jamkuras = firebaseData.intData();
    Serial.println(jamkuras);
  } else {
    Serial.println(firebaseData.errorReason());
  } 
  
  if (Firebase.getInt(firebaseData, "/jampakan1")) {
    Serial.print(" jampakan1:");
    jampakan1 = firebaseData.intData();
    Serial.println(jampakan1);
  } else {
    Serial.println(firebaseData.errorReason());
  } 

  if (Firebase.getInt(firebaseData, "/jampakan2")) {
    Serial.print("jampakan2:");
    jampakan2 = firebaseData.intData();
    Serial.println(jampakan2);
  } else {
    Serial.println(firebaseData.errorReason());
  } 

  if (Firebase.getInt(firebaseData, "/pakanmanual")) {
    Serial.print(" pakanmanual:");
    pakanmanual = firebaseData.intData();
    Serial.println(pakanmanual);
  } else {
    Serial.println(firebaseData.errorReason());
  } 

  if (Firebase.getInt(firebaseData, "/kurasmanual")) {
    Serial.print(" kurasmanual:");
    kurasmanual = firebaseData.intData();
    Serial.println(kurasmanual);
  } else {
    Serial.println(firebaseData.errorReason());
  } 


  if (Firebase.getInt(firebaseData, "/delaykatup")) {
    Serial.print(" delaykatup:");
    delaykatup = firebaseData.intData();
    Serial.println(delaykatup);
  } else {
    Serial.println(firebaseData.errorReason());
  } 
  // kirim data
  text="pakantersedia";
  if(Firebase.setInt(firebaseData, text, digitalRead(pinir))){ Serial.print(text); Serial.println(" Ok"); }
  else { Serial.print(text); Serial.println(" Failed"); }

  text="suhu";
  if(Firebase.setFloat(firebaseData, text, suhu)){ Serial.print(text); Serial.println(" Ok"); }
  else { Serial.print(text); Serial.println(" Failed"); }

  text="time";
  if(Firebase.setString(firebaseData, text, all_time)){ Serial.print(text); Serial.println(" Ok"); }
  else { Serial.print(text); Serial.println(" Failed"); }
  
}
// program utama
void program_utama(){
  int adc = analogRead(pinfloat); // baca sensor level air
  
  if(counter==0)get_firebase(); // panggi update firebase
  
  unsigned long currentMillis = millis(); // jalankan timer millis
  if (currentMillis - previousMillis >= interval) { // kondisi per 1 detik 
      previousMillis = currentMillis;
      // baca sensor suhu
      sensor.requestTemperatures();
      suhu = sensor.getTempCByIndex(0);
      // tampilkan diserial monitor
      Serial.print(" sensor level:");
      Serial.print(adc);
      Serial.print(" sensor pakan:");
      Serial.print(digitalRead(pinir));
      Serial.print(" sensor suhu:");
      Serial.print(suhu);
      // baca data waktu NTP
      baca_ntp();
      // jalankan counter sampai 10 (detik) untuk update firebase
      counter++;
      if(counter>=10) counter=0;
  // kondisi pemberian pakan sesuai jadwal pakan1
  if(jam==jampakan1&&menit==0&&detik==0){
  if(notifikasi!=1)notifikasi=1;
  Serial.println("Pakan 1 terbuka");
  digitalWrite(pinbuzzer,1); // buzzer on
  myservo.write(buka); // servo terbuka
  delay(delaykatup); // jeda
  myservo.write(tutup); // servo tertutup
  text="notifikasi"; // kirim indikator ke firebase
  if(Firebase.setString(firebaseData, text, "pemberian pakan 1")){ Serial.print(text); Serial.println(" Ok"); }
  else { Serial.print(text); Serial.println(" Failed"); }
  digitalWrite(pinbuzzer,0);   
  }
  // kondisi pemberian pakan sesuai jadwal pakan2
  if(jam==jampakan2&&menit==0&&detik==0){
  if(notifikasi!=2)notifikasi=2;
  Serial.println("Pakan 2 terbuka");
  digitalWrite(pinbuzzer,1);
  myservo.write(buka);
  delay(delaykatup);  
  myservo.write(tutup); 
  text="notifikasi";
  if(Firebase.setString(firebaseData, text, "pemberian pakan 2")){ Serial.print(text); Serial.println(" Ok"); }
  else { Serial.print(text); Serial.println(" Failed"); }
  digitalWrite(pinbuzzer,0);   
  }

  // netralkan indikator notifikasi setelag detik ke 30
  if(detik>30){
  if(notifikasi>0&&notifikasi<=3){
  notifikasi=0;
  text="notifikasi";
  if(Firebase.setString(firebaseData, text, "running")){ Serial.print(text); Serial.println(" Ok"); }
  else { Serial.print(text); Serial.println(" Failed"); }
  digitalWrite(pinbuzzer,0);  
  }
  }
  // kondisi pemberian pakan manual dari firebase
  if(pakanmanual==1&&notifikasi!=3){
  Serial.println("Pakan Manual terbuka");
  digitalWrite(pinbuzzer,1);
  myservo.write(buka);
  delay(delaykatup);  
  myservo.write(tutup); 
  notifikasi=3;
  text="notifikasi";
  if(Firebase.setString(firebaseData, text, "pemberian pakan manual")){ Serial.print(text); Serial.println(" Ok"); }
  else { Serial.print(text); Serial.println(" Failed"); }
  text="pakanmanual";
  pakanmanual=0;
  if(Firebase.setInt(firebaseData, text, pakanmanual)){ Serial.print(text); Serial.println(" Ok"); }
  else { Serial.print(text); Serial.println(" Failed"); }
  digitalWrite(pinbuzzer,0);  
  }
  // kondisi pengurasan sesuai jadwal
  if(jam==jamkuras&&hari==harikuras&&menit==0&&detik==0){
  if(notifikasi!=4)notifikasi=4;
  digitalWrite(pinrelay2,0); // pompa kuras on
  Serial.println("sedang pengurasan otomatis");
  digitalWrite(pinbuzzer,1);
  if(Firebase.setString(firebaseData, text, "sedang pengurasan otomatis")){ Serial.print(text); Serial.println(" Ok"); }
  else { Serial.print(text); Serial.println(" Failed"); }
  digitalWrite(pinbuzzer,0);    
  }
  // kondisi pengurasan secara manual dari firebase
  if(kurasmanual==1&&notifikasi!=5){
  digitalWrite(pinrelay2,0); // pompa kuras on
  Serial.println("sedang pengurasan manual");
  digitalWrite(pinbuzzer,1);
  notifikasi=5;
  text="notifikasi";
  if(Firebase.setString(firebaseData, text, "sedang pengurasan manual")){ Serial.print(text); Serial.println(" Ok"); }
  else { Serial.print(text); Serial.println(" Failed"); }
  text="kurasmanual";
  kurasmanual=0;
  if(Firebase.setInt(firebaseData, text, kurasmanual)){ Serial.print(text); Serial.println(" Ok"); }
  else { Serial.print(text); Serial.println(" Failed"); }
  digitalWrite(pinbuzzer,0);  
  }
  
  digitalWrite(D4,digitalRead(D4)^1);
  }

  if(notifikasi>=4){ // kondisi saat pengurasan
  if(digitalRead(pinrelay2)==0){ // kondisi pompa kuras on
  if(adc==0){
  Serial.println("Air bersih masuk");
  digitalWrite(pinrelay2,1); // pompa kuras off 
  digitalWrite(pinrelay1,0); // pompa air bersih on     
  }
  }

  if(digitalRead(pinrelay1)==0){ // kondisi pompa air bersih on
  if(adc>1000){
  notifikasi=0;
  Serial.println("Pengurasan selesai");
  digitalWrite(pinrelay2,1); // pompa kuras off 
  digitalWrite(pinrelay1,1); // pompa air bersih off  
  text="notifikasi"; // netralkan indikator notifikasi
  if(Firebase.setString(firebaseData, text, "running")){ Serial.print(text); Serial.println(" Ok"); }
  else { Serial.print(text); Serial.println(" Failed"); }   
  }
  }
  
  }
  
}

void setup() {
  // inisialisasis servo
  myservo.attach(pinservo);
  myservo.write(tutup);
  // inisialisasi input/output
  pinMode(D4, OUTPUT);
  pinMode(pinir, INPUT_PULLUP);
  
  pinMode(pinbuzzer, OUTPUT);
  digitalWrite(pinbuzzer,0);

  pinMode(pinrelay1, OUTPUT);
  pinMode(pinrelay2, OUTPUT);
  digitalWrite(pinrelay1, 1);
  digitalWrite(pinrelay2, 1);
  
  Serial.begin(9600);  // inisialisasi serial komunikasi ke pc
  
  Serial.println("starting");

  //  konek ke wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    
  }
  // terhubung ke wifi
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  // inisialisasi firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  // inisialisasi ntp
  timeClient.begin();
  // inisialisasi sensor
  sensor.begin();
}

void loop() {
  // panggil pogram utama di loop
  program_utama();
 
}
