#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "Enter Your SSID";
const char* password = "Enter Your PASSWORD";

// Initialize Telegram BOT
#define TELEGRAM_BOT_TOKEN "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXX"  // your Bot Token (Get from Botfather)

#define CHAT_ID "-XXXXXXXXXX"

WiFiClientSecure client;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, client);

int delayBetweenChecks = 100;
unsigned long lastTimeChecked = 0;

const int motorEnablePin = 25;  // L298N enable pin
const int motorInput1Pin = 4;  // L298N input 1 pin
const int motorInput2Pin = 5;  // L298N input 2 pin

const int MOTOR_STOP = 0;
const int MOTOR_FORWARD = 1;
const int MOTOR_BACKWARD = 2;

int motorState = MOTOR_STOP;
unsigned long motorStartTime = 0;
unsigned long motorDuration = 5000;  // 5 seconds

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));
  for (int i = 0; i < numNewMessages; i++) 
  {// If the type is a "callback_query", a inline keyboard button was pressed
    if (bot.messages[i].type ==  F("callback_query")) 
   {String text = bot.messages[i].text;
    Serial.print("Call back button pressed with text: ");
    Serial.println(text);

      if (text == F("OC")) { 
        motorState = MOTOR_FORWARD;
        motorStartTime = millis();
        }
      if (text == F("CC")){
      motorState = MOTOR_BACKWARD;
      motorStartTime = millis();
        }
      if (text == F("SC"))  {
      motorState = MOTOR_STOP; }

    } else {
      String chat_id = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;
  
      if (text == F("/options"))  
       { String keyboardJson = F("[[{ \"text\" : \"OPEN-CURTAIN\", \"callback_data\" : \"OC\" },{ \"text\" : \"CLOSE-CURTAIN\", \"callback_data\" : \"CC\" },{ \"text\" : \"STOP-CURTAIN\", \"callback_data\" : \"SC\" }]]");
        bot.sendMessageWithInlineKeyboard(chat_id, "Press The Buttons", "", keyboardJson);
      }

      if (text == F("/start")) 
      
       { bot.sendMessage(chat_id, "/options : for control your curtain\n", "Markdown");}
      
    }
   }
  }

void configureMotorPins() {
  pinMode(motorEnablePin, OUTPUT);
  pinMode(motorInput1Pin, OUTPUT);
  pinMode(motorInput2Pin, OUTPUT);
  digitalWrite(motorEnablePin, HIGH);  // Enable the motor driver
  motorState = MOTOR_STOP;
  digitalWrite(motorInput1Pin, LOW);
  digitalWrite(motorInput2Pin, LOW);
}

void setup() {
  Serial.begin(115200);
// Set WiFi to station mode and disconnect from an AP if it was Previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

 configureMotorPins();

   // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
   Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

// Serial.print("Retrieving time: ");
//   configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
//   time_t now = time(nullptr);
//   while (now < 24 * 3600)
//   {
//     Serial.print(".");
//     delay(100);
//     now = time(nullptr);
//   }
//   Serial.println(now);
  
}

void reconnectWiFi() {
  Serial.println("Attempting to reconnect to WiFi...");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi reconnected");
  } else {
    Serial.println("\nFailed to reconnect to WiFi. Please check your network credentials.");
  }
}

void controlMotor() {
  switch (motorState) {
    case MOTOR_FORWARD:
      if (millis() - motorStartTime < motorDuration) {
        digitalWrite(motorInput1Pin, HIGH);
        digitalWrite(motorInput2Pin, LOW);
      } else {
        motorState = MOTOR_STOP;
        digitalWrite(motorInput1Pin, LOW);
        digitalWrite(motorInput2Pin, LOW);
         motorStartTime = 0;  // Reset motorStartTime
      }
      break;
    case MOTOR_BACKWARD:
      if (millis() - motorStartTime < motorDuration) {
        digitalWrite(motorInput1Pin, LOW);
        digitalWrite(motorInput2Pin, HIGH);
      } else {
        motorState = MOTOR_STOP;
        digitalWrite(motorInput1Pin, LOW);
        digitalWrite(motorInput2Pin, LOW);
         motorStartTime = 0;  // Reset motorStartTime
      }
      break;
    case MOTOR_STOP:
    default:
      digitalWrite(motorInput1Pin, LOW);
      digitalWrite(motorInput2Pin, LOW);
      break;
  }
}

void loop() {
  if (millis() - lastTimeChecked > delayBetweenChecks)  
  { // getUpdates returns 1 if there is a new message from Telegram
     int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

        if (WiFi.status() != WL_CONNECTED) {
      reconnectWiFi();
    }

     while (numNewMessages) {
                         Serial.println("got response");
                         handleNewMessages(numNewMessages);
       numNewMessages = bot.getUpdates(bot.last_message_received + 1);
                         }

    lastTimeChecked = millis();
  }
  controlMotor();
}
