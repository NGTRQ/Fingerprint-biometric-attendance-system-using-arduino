#include <EEPROM.h>
#include <Wire.h>
#include <RTClib.h>
#include <FPS_GT511C3.h> // Make sure to include the correct FPS library

RTC_DS3231 rtc;

FPS_GT511C3 fps; // Use the appropriate class instance for your FPS sensor

#define register_back 14
#define delete_ok 15
#define forward 16
#define reverse 17
#define match 5
#define indFinger 7
#define buzzer 6

#define records 10 // 10 for 10 user

int userId[records]; // Change user array to userId

DateTime now;

void setup()
{
    delay(1000);
    Serial.begin(9600);
    pinMode(register_back, INPUT_PULLUP);
    pinMode(forward, INPUT_PULLUP);
    pinMode(reverse, INPUT_PULLUP);
    pinMode(delete_ok, INPUT_PULLUP);
    pinMode(match, INPUT_PULLUP);
    pinMode(buzzer, OUTPUT);
    pinMode(indFinger, OUTPUT);
    digitalWrite(buzzer, LOW);

    if (!rtc.begin())
        Serial.println("Couldn't find RTC");

    if (rtc.lostPower())
    {
        Serial.println("RTC is NOT running!");
        DateTime compileTime = DateTime(F(__DATE__), F(__TIME__));
        Serial.print("Comp. Time: ");
        Serial.println(compileTime.timestamp());
        rtc.adjust(compileTime);
    }

    Serial.println("Press Match to Start System");
    delay(3000);

    for (int i = 0; i < records; i++)
    {
        userId[i] = EEPROM.read(1000 + i);
    }

    digitalWrite(indFinger, HIGH);
}

void loop()
{
    now = rtc.now();
    Serial.print("Time: ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.print(" ");
    Serial.print("Date: ");
    Serial.print(now.day(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.year(), DEC);
    Serial.println(" ");
    delay(500);
    int result = getFingerprintIDez();
    if (result > 0)
    {
        digitalWrite(indFinger, LOW);
        digitalWrite(buzzer, HIGH);
        delay(100);
        digitalWrite(buzzer, LOW);
        Serial.println("ID: " + String(result));
        Serial.println("Please Wait....");
        delay(1000);
        attendance(result);
        Serial.println("Attendance Registered");
        delay(1000);
        digitalWrite(indFinger, HIGH);
    }
    // checkKeys(); // You might want to uncomment and implement this function
    delay(300);
}

void attendance(int id)
{
    int user = 0, eepLoc = 0;
    if (id >= 1 && id <= records)
    {
        eepLoc = (id - 1) * 7;
        user = userId[id - 1]++;
    }

    int eepIndex = eepLoc;

    EEPROM.write(eepIndex++, now.hour());
    EEPROM.write(eepIndex++, now.minute());
    EEPROM.write(eepIndex++, now.second());
    EEPROM.write(eepIndex++, now.day());
    EEPROM.write(eepIndex++, now.month());
    EEPROM.write(eepIndex++, now.year() >> 8);
    EEPROM.write(eepIndex++, now.year());

    for (int i = 0; i < records; i++)
    {
        EEPROM.write(1000 + i, userId[i]);
    }
}

int getFingerprintIDez() {
    // Try to capture a fingerprint
    int p = fps.CaptureFinger();

    if (p != FPS_GT511C3::FINGERPRINT_OK) {
        Serial.println("Failed to capture fingerprint");
        return -1;
    }

    // Try to identify the captured fingerprint
    p = fps.Identify1_N();

    if (p == FPS_GT511C3::FINGERPRINT_OK) {
        // Fingerprint matched successfully
        Serial.println("Fingerprint matched, ID #" + String(fps.fingerID));
        return fps.fingerID;
    } else {
        // Fingerprint not recognized
        Serial.println("Fingerprint not recognized");
        return -1;
    }
}

