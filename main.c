#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "RTClib.h"
#include <cmath>
#include <LiquidCrystal_I2C.h>
#include <WiFiClient.h>
#include "ThingSpeak.h"
#include <ESP_Mail_Client.h>
#define seaLevelPressure_hPa 1013.25
#define DHTPIN 14 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 
#define TRIGPIN    27  
#define ECHOPIN    26  
#define PIN_CamBienMua 4 
//Mail 
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The sign in credentials */
#define AUTHOR_EMAIL "kaztji28@gmail.com"
#define AUTHOR_PASSWORD "qxbdgyoohpkcmqsh"

/* Recipient's email*/
#define RECIPIENT_EMAIL "vanquang17th@gmail.com"
/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);
int emptyTankDistance = 180 ;  // empty tank
int fullTankDistance =  30 ;  // full tank
bool isRaining = false; // intial rain value
float duration;
float distance;
int waterLevelPer;
unsigned long lastDistanceCheckTime = 0; // Thời điểm cuối cùng kiểm tra khoảng cách
float initialDistance = 0; // Giá trị khoảng cách ban đầu khi bắt đầu mưa
WiFiClient  client;
char ssid[] = "Gia Bao";
char pass[] = "29122003";
unsigned long myChannelNumber = 2304721;
const char * myWriteAPIKey = "6KGNFPZI9CD5WO37";
Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHTTYPE);
RTC_DS1307 RTC;
// set the LCD number of columns and rows
int lcdColumns = 20;
int lcdRows = 4;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
void measureDistance() // main calculation function
{
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(20);
  digitalWrite(TRIGPIN, LOW);
  isRaining = digitalRead(PIN_CamBienMua); 
  duration = pulseIn(ECHOPIN, HIGH); // Xác định khoảng cách từ thời gian đo được
  distance = ((duration / 2) * 0.343); // mm
 }
 void dht_sensor()
{
  //read temperature and humidity
  float t = dht.readTemperature();
  float h = dht.readHumidity();
   if (isnan(h) || isnan(t)) 
   {
    Serial.println("Failed to read from DHT sensor!");
   }
   else
   {
    Serial.print("Temperature = ");
    Serial.print(dht.readTemperature());
    Serial.println(" C");
    Serial.print("Humidity = ");
    Serial.print(dht.readHumidity());
    Serial.println(" %");
    Serial.println();
    ThingSpeak.writeField(myChannelNumber, 1, t, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 2, h, myWriteAPIKey);
   }
}
void bmp_sensor()
{ 
  float a = bmp.readAltitude();
  float p = bmp.readPressure();
  if (isnan(a) || isnan(p)) 
  {
    Serial.println("Failed to read from bmp sensor!");
  }
  else
  {
    Serial.print("\nAltitude = ");
    Serial.print(abs(bmp.readAltitude()));
    Serial.println(" m");
    
    Serial.print("Pressure = ");
    Serial.print((bmp.readPressure())/100);
    Serial.println(" hPa");
    Serial.println();
    ThingSpeak.writeField(myChannelNumber, 3, p, myWriteAPIKey);
  }
}
void RTC_sensor()
{
  DateTime now = RTC.now();
  lcd.setCursor(0, 0);
  lcd.print(now.day());
  lcd.print('/');
  lcd.print(now.month());
  lcd.print('/');
  lcd.print(now.year());
  lcd.print(' ');
  lcd.print(now.hour());
  lcd.print(':');
  lcd.print(now.minute());
  lcd.print(':');
  lcd.print(now.second());
}
void Zambretti_Algorithm()
{
  float P=0;
  float z=0;
  DateTime now = RTC.now();
  int curr_P = (bmp.readPressure())/100;
  int prev_P = curr_P;
  float h = abs(bmp.readAltitude());
  float T = dht.readTemperature();
  P = curr_P * pow((1 - ((0.0065*h)/(T+(0.0065*h)+273.15))), -5.257);
  P = round(P);
  Serial.print("\nP :");
  Serial.print(P);
  //calculate Z - Forecast number
		//pressure trend
		if(curr_P < prev_P) //falling
    {				
			z = 127 - 0.12*P;
			if(now.month() >= 6 && now.month() < 9)
      { 	// pressure falling & summer
				z = z - 1;
			}
		}
		else if(curr_P == prev_P)
    {			// pressure steady
			z = 144 - 0.13*P;
		}
		else if(curr_P > prev_P)
    {			// pressure rising
			z = 185 - 0.16*P;
			if(now.month() >= 12 && now.month() < 3)
      {	// pressure rising & winter
				z = z + 1;
			}
		}
		z = round(z);
    Serial.println("\nZ:");
    Serial.println(z);
		switch(int(z) ){
        case 1: 
        lcd.setCursor(0, 1);
        lcd.print("Thoi tiet tot");
        Serial.println("\n Thời tiết tốt");
           break;
				case 2:
        lcd.setCursor(0, 1);
				lcd.print("Thoi tiet on dinh");
        Serial.println("Thời tiết ổn định");
           break;
				break;
				case 3:
        lcd.setCursor(0, 1);
				lcd.print("Thoi tiet tot");
        Serial.println("Thời tiết tốt, trở nên ổn định hơn");
				break;
				case 4:
        lcd.setCursor(0, 1); 
				lcd.print("Thoi tiet gan mua");
        Serial.println("Thời tiết khá ổn, có mưa sau");
				break;
				case 5:
        lcd.setCursor(0, 1);
				lcd.print("Mua rao");
        Serial.println("Mưa rào, thời tiết trở nên bất ổn hơn");
				break;
				case 6:
        lcd.setCursor(0, 1);
				lcd.print("Thoi tiet bat on,mua");
        Serial.println("Thời tiết bất ổn, mưa muộn");
				break;
				case 7:
        lcd.setCursor(0, 1);
				lcd.print("Mua, luc sau te hon");
        Serial.println("Có lúc mưa, lúc sau tệ hơn");
				break;
				case 8:
        lcd.setCursor(0, 1);
				lcd.print("Co luc mua, te hon");
        Serial.println("Có lúc mưa, trở nên rất bất ổn");
				break;
				case 9:
        lcd.setCursor(0, 1);
				lcd.print("Rat bat on, mua");
        Serial.println("Rất bất ổn, mưa");
				break;
				case 10:
        lcd.setCursor(0, 1);
				lcd.print("Thoi tiet tot");
        Serial.println("Thời tiết tốt");
				break;
				case 11:
        lcd.setCursor(0, 1);
				lcd.print("Thoi tiet on dinh");
        Serial.println("Thời tiết ổn định");
				break;
				case 12:
        lcd.setCursor(0, 1);
				lcd.print("Co the mua rao");
        Serial.println("Thời tiết ổn định, có thể mưa rào");
				break;
				case 13:
        lcd.setCursor(0, 1);
				lcd.print("Co the mua rao");
        Serial.println("Khá ổn, có thể mưa rào");
				break;
				case 14:
        lcd.setCursor(0, 1);
				lcd.print("Mua rao, troi sang");
        Serial.println("Có thể mưa rào, khoảng sáng");
				break;
				case 15:
        lcd.setCursor(0, 1);
				lcd.print("Thinh thoang mua");
        Serial.println("Có thể thay đổi, thỉnh thoảng mưa");
				break;
				case 16:
        lcd.setCursor(0, 1);
				lcd.print("Bat on, co luc mua");
        Serial.println("Bất ổn, có lúc mưa");
				break;
				case 17:
        lcd.setCursor(0, 1);
				lcd.print("Mua thuong xuyen");
        Serial.println("Mưa theo chu kỳ thường xuyên");
				break;
				case 18:
        lcd.setCursor(0, 1);
				lcd.print("Rat bat on, mua");
        Serial.println("Rất bất ổn, mưa");
				break;
				case 19:
        lcd.setCursor(0, 1);
				lcd.print("Bao, mua nhieu");
        Serial.println("Bão, mưa nhiều");
				break;
				case 20:
        lcd.setCursor(0, 1);
				lcd.print("Thoi tiet tot");
        Serial.println("Thời tiết tốt");
				break;
				case 21:
        lcd.setCursor(0, 1);
				lcd.print("Thoi tiet on dinh");
        Serial.println("Thời tiết ổn định");
				break;
				case 22:
        lcd.setCursor(0, 1);
				lcd.print("Tro nen tot hon");
        Serial.println("Thời tiết trở nên tốt hơn");
				break;
				case 23:
        lcd.setCursor(0, 1);
				lcd.print("Troi dang cai thien");
        Serial.println("Khá ổn, đang cải thiện");
				break;
				case 24:
        lcd.setCursor(0, 1);
				lcd.print("Co the mua rao som");
        Serial.println("Khá ổn, có thể mưa rào sớm");
				break;
				case 25:
        lcd.setCursor(0, 1);
				lcd.print("Co mua som");
        Serial.println("Có mưa sớm, cải thiện");
				break;
				case 26:
        lcd.setCursor(0, 1);
				lcd.print("Thoi tiet thay doi");
        Serial.println("Thời tiết có thay đổi");
				break;
				case 27:
        lcd.setCursor(0, 1);
				lcd.print("Kha bat on");
        Serial.println("Khá bất ổn");
				break;
				case 28:
        lcd.setCursor(0, 1);
				lcd.print("Dang cai thien");
        Serial.println("Chưa ổn định, có thể cải thiện");
				break;
				case 29:
        lcd.setCursor(0, 1);
				lcd.print("Chua on time ngan");
        Serial.println("Chưa ổn định, khoảng thời gian ngắn");
				break;
				case 30:
        lcd.setCursor(0, 1);
				lcd.print("Co luc tot hon");
        Serial.println("Rất bất ổn, có lúc tốt hơn");
				break;
				case 31:
        lcd.setCursor(0, 1);
				lcd.print("Bao,co the cai thien");
        Serial.println("Bão, có thể cải thiện");
				break;
				case 32:
        lcd.setCursor(0, 1);
				lcd.print("Bao, mua nhieu");
        Serial.println("Bão, mưa nhiều");
				break;
				default:
        lcd.setCursor(0, 1);
				lcd.print("Invalid Forcase");
        Serial.println("Invalid Forcase");
          break;
			}
		    prev_P = curr_P;
        Serial.print("\nPrev_P :");
        Serial.print(prev_P);
        Serial.print(" hPa");
}
void setup() 
{
  Serial.begin(115200);
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
   dht.begin();

  pinMode(ECHOPIN, INPUT);
  pinMode(TRIGPIN, OUTPUT);
  pinMode(PIN_CamBienMua, INPUT);
  WiFi.begin(ssid, pass);
  ThingSpeak.begin(client);
  delay(100);
  if (!bmp.begin()) 
  {
  Serial.println("BMP180 Not Found. CHECK CIRCUIT!");
  while (1) {}
  }
  // RTC sensor
  if (!RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void warning_rain()
{
  SMTP_Message message;
  unsigned long currentTime = millis(); // Lấy thời gian hiện tại
  if (isRaining == LOW)
  {
    if (initialDistance == 0) 
    { // Nếu đây là lần đầu tiên mưa
      initialDistance = distance; // Lưu trữ khoảng cách ban đầu
      lastDistanceCheckTime = currentTime;
    }
    float distanceChange = abs(initialDistance - distance); // Tính sự thay đổi khoảng cách kể từ khi bắt đầu mưa
    Serial.println(" đang mưa");
    Serial.print("mực nước dâng :");
    Serial.print(distanceChange);
    Serial.println(" mm");
    lcd.setCursor(0,2);
    lcd.print("It's raining");
    lcd.setCursor(0,3);
    lcd.print("Rain water rise:");
    lcd.print(round(distanceChange));
    ThingSpeak.writeField(myChannelNumber, 5, distanceChange, myWriteAPIKey);
    if (labs(currentTime - lastDistanceCheckTime) <= 1800000) 
    { 
     if (distanceChange >= 30) 
     { // Nếu khoảng cách giảm 30mm
      Serial.println("mực nước dâng vừa có thể lụt");
      lcd.setCursor(0,3);
      lcd.print("Red warning         ");
            // Send an email
      Session_Config config;

  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  SMTP_Message message;

  message.sender.name = F("ESP");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = F("Warning: Water level rising rapidly");
  message.addRecipient(F("Sara"), RECIPIENT_EMAIL);
    
  //Send raw text message
  String textMsg = "The water level has increased more than 30mm.";
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  if (!smtp.connect(&config)){
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn()){
    Serial.println("\nNot yet logged in.");
  }
  else{
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }

  if (!MailClient.sendMail(&smtp, &message))
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

     }
     else if(distanceChange >=20)
     {
      Serial.println("mực nước dâng nhanh có the ngap ung");
      lcd.setCursor(0,3);
      lcd.print("Warning             ");
           // Send an email
      Session_Config config;

  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  SMTP_Message message;

  message.sender.name = F("ESP");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = F("Warning: Water level rising rapidly");
  message.addRecipient(F("Sara"), RECIPIENT_EMAIL);
    
  //Send raw text message
  String textMsg = "The water level has increased more than 20mm.";
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;
  
  if (!smtp.connect(&config)){
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn()){
    Serial.println("\nNot yet logged in.");
  }
  else{
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }

  if (!MailClient.sendMail(&smtp, &message))
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

     }}}
    // Cập nhật giá trị và thời gian cho lần kiểm tra tiếp theo
    // previousDistance = distance;
     //lastDistanceCheckTime = currentTime;}
  else
  {
    initialDistance = 0;
   if (distance > fullTankDistance && distance < emptyTankDistance) 
   {
    waterLevelPer = map((int)distance, emptyTankDistance, fullTankDistance, 0, 100);
    // Hiển thị dữ liệu lên màn hình OLED và gửi lên Blynk app
    ThingSpeak.writeField(myChannelNumber, 4, waterLevelPer, myWriteAPIKey);
    lcd.setCursor(0,2);
    lcd.print("Percent: ");
    lcd.print(waterLevelPer);
    lcd.print( "%");
    lcd.setCursor(0,3);
    lcd.print("                    ");
    Serial.print("Percent: ");
    Serial.print(waterLevelPer);
    Serial.print("%");        
  }
 } 
}
void loop() 
{
   RTC_sensor();
   bmp_sensor();
   dht_sensor();
   Zambretti_Algorithm();
   measureDistance();
   warning_rain();
}
