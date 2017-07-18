#include <DuoBLE.h>

SYSTEM_MODE(MANUAL); // Disable WiFi

const char * const deviceName = "wils";

BLEService myService("a337c90b-4e78-469e-905f-8a3248257e45");
BLECharacteristic lightStatusChar("a02f", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_NOTIFY, 1, 1);
BLECharacteristic rgbChar("b13a", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE, 3, 3);
BLECharacteristic alarmOnTimeChar("c25d", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE, 1, 1);
BLECharacteristic alarmOnChar("d95e", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE, 1, 1);
BLECharacteristic alarmOffTimeChar("e67b", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE, 1, 1);
BLECharacteristic alarmOffChar("f22f", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE, 1, 1);
BLECharacteristic timeChar("0ad7", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE, 4, 4);
BLECharacteristic onAtChar("1be8", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE, 4, 4);
BLECharacteristic offAtChar("2cf9", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE, 4, 4);
BLECharacteristic rgbDefaultChar("3fa1", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE, 3, 3);

// Define where the button is connected and it's state
const int onButton = D1;
const int offButton = D2;
bool onButtonPressed = false;
bool offButtonPressed = false;
bool lightOn = false;

//default colors
int defaultColor[] = {255, 255, 255};
//int defaultColor[] = {255,255,255};
int currentColor[] = {0, 0, 0};
//int currentColor[] = {defaultColor[0],defaultColor[1],defaultColor[2]};

//fade stuff
int fadeInterval = 100;
int fadeDuration  = 5000;
int deltaColor[] = {0, 0, 0};
int endColor[] = {0, 0, 0};

//alarm stuff
unsigned long time1;
int fakeCurrentTime = 1491974577;
//int fakeCurrentTime = 0;
int onAlarmTime = 0;
int offAlarmTime = 0;
bool alarmTurnsOn = false;
bool alarmTurnsOff = false;

//on/off at timer stuff
bool onAtIsOn = true;
bool offAtIsOn = true;
int onAtHour = 5;
int onAtMinute = 23;
int onAtSecond = 30;
int offAtHour = 5;
int offAtMinute = 23;
int offAtSecond = 50;

void fadeMethod() {
  //  Serial.println("fademethod");
  int red = currentColor[0] + deltaColor[0];
  int green = currentColor[1] + deltaColor[1];
  int blue = currentColor[2] + deltaColor[2];
  currentColor[0] = red;
  currentColor[1] = green;
  currentColor[2] = blue;
  //  Serial.print("red: ");
  //  Serial.println(red);
  //  Serial.print("green: ");
  //  Serial.println(green);
  //  Serial.print("blue: ");
  //  Serial.println(blue);
  RGB.color(red, green, blue);
}

void onInTimerMethod() {
  crossFade(defaultColor[0], defaultColor[1], defaultColor[2]);
}

void offInTimerMethod() {
  crossFade(0, 0, 0);
}



//Timers
Timer fader(fadeInterval, fadeMethod);
Timer fadeStop(fadeDuration, stopFade, true);
int checkButtonInterval = 100;
Timer checkOnButton(checkButtonInterval, checkOnButtonMethod);
Timer checkOffButton(checkButtonInterval, checkOffButtonMethod);
Timer onInTimer(1000, onInTimerMethod, true);
Timer offInTimer(1000, offInTimerMethod, true);
Timer onAtTimer(1000, onAtTimerMethod);


void sendLightStatus() {
  byte value[1];
  value[0] = lightOn;
  lightStatusChar.setValue(value, 1);
  lightStatusChar.sendNotify();
}


void onAtTimerMethod() {
//  Serial.print("The current time is : ");
//  Serial.println(Time.timeStr());
  if (onAtHour == Time.hour()) {
    if (onAtMinute == Time.minute()) {
      if (onAtSecond == Time.second()) {
        crossFade(defaultColor[0], defaultColor[1], defaultColor[2]);
      }
    }
  }
  if (offAtHour == Time.hour()) {
    if (offAtMinute == Time.minute()) {
      if (offAtSecond == Time.second()) {
        crossFade(0, 0, 0);
      }
    }
  }
}


void stopFade() {
  Serial.println("fadestop");
  fader.stop();
  RGB.color(endColor[0], endColor[1], endColor[2]);
  for (int i = 0; i < 3; i++) {
    currentColor[i] = endColor[i];
  }
  if(endColor[0] == 0 && endColor[1] == 0 && endColor[2] == 0){
    lightOn = false;
  }else{
    lightOn = true;
  }
  sendLightStatus();
}

void crossFade(int r, int g, int b) {
  //stops timers in case they are running
  fader.stop();
  fadeStop.stop();

  //if the current color is off set LightOn 
  if(currentColor[0] == 0 && currentColor[1] == 0 && currentColor[2] == 0 && (r > 0 || g > 0 || b >0)){
    lightOn = true;
    sendLightStatus();
  }

  endColor[0] = r;
  endColor[1] = g;
  endColor[2] = b;
  int overallDeltaColor[] = {0, 0, 0};

  //overallDeltaColor[i] will be negative if you are fading to a lower r,g,or b value
  for (int i = 0; i < 3; i++) {
    overallDeltaColor[i] = endColor[i] - currentColor[i];
  }
  //fill the delta color array so you know how much to fade every interval
  for (int i = 0; i < 3; i++) {
    deltaColor[i] = (int)( ( ((float)overallDeltaColor[i]) / (float)fadeDuration) * (float)fadeInterval);

  }
  //start the fade
  fader.reset();
  fadeStop.reset();
}


//void printTimeMethod() {
//  Serial.print("The current time is : ");
//  Serial.println(Time.timeStr());
//}
//
//Timer printTimer(1000, printTimeMethod);

void checkOnButtonMethod() {
  if (digitalRead(onButton) == 0) {
    Serial.println("ON still pressed");
    onButtonPressed = true;
  } else {
    Serial.println("ON not pressed");
    checkOnButton.stopFromISR();
    onButtonPressed = false;

    crossFade(defaultColor[0], defaultColor[1], defaultColor[2]);
  }
}

void checkOffButtonMethod() {
  if (digitalRead(offButton) == 0) {
    Serial.println("OFF still pressed");
    offButtonPressed = true;
  } else {
    Serial.println("OFF not pressed");
    checkOffButton.stopFromISR();
    offButtonPressed = false;
    crossFade(0, 0, 0);
  }
}

void onButtonChanged() {
  checkOnButton.startFromISR();
  onButtonPressed = (digitalRead(onButton) == 0);
  if (onButtonPressed) {
    lightOn = true;
  }
  Serial.println("ON initial press");
}


void offButtonChanged() {
  checkOffButton.startFromISR();
  offButtonPressed = (digitalRead(offButton) == 0);
  Serial.println("OFF initial press");
  if (offButtonPressed) {
    lightOn = false;
  }
}

void rgbCharCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  const char *reasons[] = {"PREREAD",  "POSTREAD", "POSTWRITE", "NOTIFICATIONS_ENABLED", "NOTIFICATIONS_DISABLED",  "INDICATIONS_ENABLED", "INDICATIONS_DISABLED"};
  Serial.print("Read/Write Characteristic; Reason: ");
  Serial.println(reasons[reason]);
  if (reason == PREREAD) {
    // Update the data before it's read
    byte value[3];
    value[0] = currentColor[0];
    value[1] = currentColor[1];
    value[2] = currentColor[2];
    rgbChar.setValue(value, 3);
  }

  if (reason == POSTWRITE) {
    byte value[3];
    int bytes = rgbChar.getValue(value, 3);
    //      int tempColors[] = {0,0,0};
    for (int i = 0; i < bytes; i++) {
      currentColor[i] = value[i];
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(value[i]);
    }
    crossFade(currentColor[0], currentColor[1], currentColor[2]);
    //    if (lightOn) {
    //      crossFade(defaultColor[0], defaultColor[1], defaultColor[2]);
    //    }
  }




}

void lightStatusCharCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  const char *reasons[] = {"PREREAD",  "POSTREAD", "POSTWRITE", "NOTIFICATIONS_ENABLED", "NOTIFICATIONS_DISABLED",  "INDICATIONS_ENABLED", "INDICATIONS_DISABLED"};
  Serial.print("Read/Write Characteristic; Reason: ");
  Serial.println(reasons[reason]);

  if (reason == PREREAD) {
    // Update the data before it's read
    byte value[1];
    value[0] = lightOn;
    lightStatusChar.setValue(value, 1);
  }

  if (reason == POSTWRITE) {
    byte value[20];
    int bytes = lightStatusChar.getValue(value, 20);
    int temp = 0;
    for (int i = 0; i < bytes; i++) {
      temp += value[i];
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(value[i]);
    }
    switch (temp) {
      case 0:
        lightOn = false;
        crossFade(0, 0, 0);
        break;
      case 1:
        lightOn = true;
        crossFade(defaultColor[0], defaultColor[1], defaultColor[2]);

        break;
      case 2:
        lightOn = false;
        fader.stop();
        fadeStop.stop();
        RGB.color(0, 0, 0);
        currentColor[0] = 0;
        currentColor[1] = 0;
        currentColor[2] = 0;
        sendLightStatus();
        break;
      case 3:
        lightOn = true;
        fader.stop();
        fadeStop.stop();
        RGB.color(defaultColor[0], defaultColor[1], defaultColor[2]);
        currentColor[0] = defaultColor[0];
        currentColor[1] = defaultColor[1];
        currentColor[2] = defaultColor[2];
        sendLightStatus();
        break;
    }

    Serial.println(temp);

  }

  if (reason == NOTIFICATIONS_ENABLED) {
    byte value[1];
    value[0] = lightOn;
    lightStatusChar.setValue(value, 1);
    lightStatusChar.sendNotify();
  }
}

void alarmOffTimeCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  const char *reasons[] = {"PREREAD",  "POSTREAD", "POSTWRITE", "NOTIFICATIONS_ENABLED", "NOTIFICATIONS_DISABLED",  "INDICATIONS_ENABLED", "INDICATIONS_DISABLED"};
  Serial.print("Read/Write Characteristic; Reason: ");
  Serial.println(reasons[reason]);

  if (reason == PREREAD) {
    byte alarmValue[1];
    alarmValue[0] = offAlarmTime/1000;
    alarmOffTimeChar.setValue(alarmValue, 1);
  }

  if (reason == POSTWRITE) {
    byte value[1];
    alarmOffTimeChar.getValue(value, 1);
    int i = value[0];
    Serial.print("alarm off time value: " );
    Serial.println(i);
    offAlarmTime = i*1000;
  }

}

void alarmOffCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  const char *reasons[] = {"PREREAD",  "POSTREAD", "POSTWRITE", "NOTIFICATIONS_ENABLED", "NOTIFICATIONS_DISABLED",  "INDICATIONS_ENABLED", "INDICATIONS_DISABLED"};
  Serial.print("Read/Write Characteristic; Reason: ");
  Serial.println(reasons[reason]);

  if (reason == PREREAD) {
    byte value[1];
    value[0] = alarmTurnsOff;
    alarmOffChar.setValue(value, 1);
  }

  if (reason == POSTWRITE) {
    byte value[1];
    alarmOffChar.getValue(value, 1);
    int n = value[0];
    Serial.print("write to alarm off callback: ");
    Serial.println(n);
    //whether or not there is an off light timer
    if (n == 0) {
      alarmTurnsOff = false;
      offInTimer.reset();
      offInTimer.stop();
    } else {
      alarmTurnsOff = true;
      offInTimer.changePeriod(offAlarmTime);
      offInTimer.start();
    }
  }

}

void alarmOnTimeCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  const char *reasons[] = {"PREREAD",  "POSTREAD", "POSTWRITE", "NOTIFICATIONS_ENABLED", "NOTIFICATIONS_DISABLED",  "INDICATIONS_ENABLED", "INDICATIONS_DISABLED"};
  Serial.print("Read/Write Characteristic; Reason: ");
  Serial.println(reasons[reason]);

  if (reason == PREREAD) {
    byte alarmValue[1];
    alarmValue[0] = onAlarmTime/1000;
    alarmOnTimeChar.setValue(alarmValue, 1);
  }

  if (reason == POSTWRITE) {
    byte value[1];
    alarmOnTimeChar.getValue(value, 1);
    int i = value[0];
    Serial.print("write to alarm on time value: " );
    Serial.println(i);
    onAlarmTime = i*1000;
  }

}

void alarmOnCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  const char *reasons[] = {"PREREAD",  "POSTREAD", "POSTWRITE", "NOTIFICATIONS_ENABLED", "NOTIFICATIONS_DISABLED",  "INDICATIONS_ENABLED", "INDICATIONS_DISABLED"};
  Serial.print("Read/Write Characteristic; Reason: ");
  Serial.println(reasons[reason]);

  if (reason == PREREAD) {
    byte value[1];
    value[0] = alarmTurnsOn;
    alarmOnChar.setValue(value, 1);
  }

  if (reason == POSTWRITE) {
    byte value[1];
    alarmOnChar.getValue(value, 1);
    int n = value[0];
    Serial.print("write to alarm on  callback: ");
    Serial.println(n);
    //whether or not there is an on light timer
    if (n == 0) {
      alarmTurnsOn = false;
      onInTimer.reset();
      onInTimer.stop();
    } else {
      alarmTurnsOn = true;
      onInTimer.changePeriod(onAlarmTime);
      onInTimer.start();
    }
  }

}

void timeCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  const char *reasons[] = {"PREREAD",  "POSTREAD", "POSTWRITE", "NOTIFICATIONS_ENABLED", "NOTIFICATIONS_DISABLED",  "INDICATIONS_ENABLED", "INDICATIONS_DISABLED"};
  Serial.print("Read/Write Characteristic; Reason: ");
  Serial.println(reasons[reason]);

  if (reason == PREREAD) {
    byte value[4];
    value[0] = fakeCurrentTime >> 24;
    value[1] = fakeCurrentTime >> 16;
    value[2] = fakeCurrentTime >> 8;
    value[3] = fakeCurrentTime;
    alarmOnTimeChar.setValue(value, 4);
  }

  if (reason == POSTWRITE) {
    byte value[4];
    timeChar.getValue(value, 4);
    int i = (value[0] << 24) | (value[1] << 16) | (value[2] << 8) | (value[3] << 0);
    Serial.print("current time value: " );
    Serial.println(i);
    fakeCurrentTime = i;
    Time.setTime(fakeCurrentTime);
  }

}

void onAtCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  const char *reasons[] = {"PREREAD",  "POSTREAD", "POSTWRITE", "NOTIFICATIONS_ENABLED", "NOTIFICATIONS_DISABLED",  "INDICATIONS_ENABLED", "INDICATIONS_DISABLED"};
  Serial.print("Read/Write Characteristic; Reason: ");
  Serial.println(reasons[reason]);

  if (reason == PREREAD) {
    byte onAtValue[4];
    onAtValue[0] = onAtIsOn;
    onAtValue[1] = onAtHour;
    onAtValue[2] = onAtMinute;
    onAtValue[3] = onAtSecond;
    onAtChar.setValue(onAtValue, 4);
  }

  if (reason == POSTWRITE) {
    byte value[4];
    onAtChar.getValue(value, 4);
    onAtIsOn = (value[0] != 0) ? true : false;
    onAtHour = value[1];
    onAtMinute = value[2];
    onAtSecond = value[3];
    Serial.print("On at is On: ");
    Serial.println(onAtIsOn);
    Serial.print("on at Hour: ");
    Serial.println(onAtHour);
    Serial.print("on at Minute: ");
    Serial.println(onAtMinute);
    Serial.print("on at Second: ");
    Serial.println(onAtSecond);
  }
}

void offAtCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  const char *reasons[] = {"PREREAD",  "POSTREAD", "POSTWRITE", "NOTIFICATIONS_ENABLED", "NOTIFICATIONS_DISABLED",  "INDICATIONS_ENABLED", "INDICATIONS_DISABLED"};
  Serial.print("Read/Write Characteristic; Reason: ");
  Serial.println(reasons[reason]);

  if (reason == PREREAD) {
    byte offAtValue[4];
    offAtValue[0] = offAtIsOn;
    offAtValue[1] = offAtHour;
    offAtValue[2] = offAtMinute;
    offAtValue[3] = offAtSecond;
    offAtChar.setValue(offAtValue, 4);
  }

  if (reason == POSTWRITE) {
    byte value[4];
    offAtChar.getValue(value, 4);
    offAtIsOn = (value[0] != 0) ? true : false;
    offAtHour = value[1];
    offAtMinute = value[2];
    offAtSecond = value[3];

  }
}

void rgbDefaultCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  const char *reasons[] = {"PREREAD",  "POSTREAD", "POSTWRITE", "NOTIFICATIONS_ENABLED", "NOTIFICATIONS_DISABLED",  "INDICATIONS_ENABLED", "INDICATIONS_DISABLED"};
  Serial.print("Read/Write Characteristic; Reason: ");
  Serial.println(reasons[reason]);
  if (reason == PREREAD) {
    // Update the data before it's read
    byte value[3];
    value[0] = defaultColor[0];
    value[1] = defaultColor[1];
    value[2] = defaultColor[2];
    rgbDefaultChar.setValue(value, 3);
  }

  if (reason == POSTWRITE) {
    byte value[3];
    int bytes = rgbDefaultChar.getValue(value, 3);
    //      int tempColors[] = {0,0,0};
    for (int i = 0; i < bytes; i++) {
      defaultColor[i] = value[i];
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(value[i]);
    }
    
  }
}


void setup() {
  delay(2000);
  Serial.begin(9600);
  pinMode(onButton, INPUT_PULLUP);
  pinMode(offButton, INPUT_PULLUP);
  RGB.control(true);
  attachInterrupt(onButton, onButtonChanged, FALLING);
  attachInterrupt(offButton, offButtonChanged, FALLING);
  Time.setTime(fakeCurrentTime);
  time1 = millis();
  //  printTimer.start();
//  onAtTimer.start();

  // Setup the Bluetooth Service: Initialize the value of the characteristic and add a callback function
  byte initValue[1] = {0}; // Initially not pressed
  lightStatusChar.setValue(initValue, 1);
  lightStatusChar.setCallback(lightStatusCharCallback);

  //rgbChar setup
  byte rgbValue[3] = {currentColor[0], currentColor[1], currentColor[2]};
  rgbChar.setValue(rgbValue, 3);
  rgbChar.setCallback(rgbCharCallback);

  //alarmOnTimeChar setup
  byte alarmOnValue[1];
  alarmOnValue[0] = onAlarmTime/1000;
  alarmOnTimeChar.setValue(alarmOnValue, 1);
  alarmOnTimeChar.setCallback(alarmOnTimeCallback);

  //alarmOnChar setup
  byte alarmOnValue1[] = {0};
  alarmOnChar.setValue(alarmOnValue1, 1);
  alarmOnChar.setCallback(alarmOnCallback);

  //alarmOffTimeChar setup
  byte alarmOffValue[1];
  alarmOffValue[0] = offAlarmTime/1000;
  alarmOffTimeChar.setValue(alarmOffValue, 1);
  alarmOffTimeChar.setCallback(alarmOffTimeCallback);

  //alarmOffChar setup
  byte alarmOffValue1[] = {0};
  alarmOffChar.setValue(alarmOffValue1, 1);
  alarmOffChar.setCallback(alarmOffCallback);

  //timeChar setup
  byte timeValue[4];
  timeValue[0] = fakeCurrentTime >> 24;
  timeValue[1] = fakeCurrentTime >> 16;
  timeValue[2] = fakeCurrentTime >> 8;
  timeValue[3] = fakeCurrentTime;
  timeChar.setValue(timeValue, 4);
  timeChar.setCallback(timeCallback);

  //onAtChar setup
  byte onAtValue[4];
  onAtValue[0] = onAtIsOn;
  onAtValue[1] = onAtHour;
  onAtValue[2] = onAtMinute;
  onAtValue[3] = onAtSecond;
  onAtChar.setValue(onAtValue, 4);
  onAtChar.setCallback(onAtCallback);

  byte offAtValue[4];
  offAtValue[0] = offAtIsOn;
  offAtValue[1] = offAtHour;
  offAtValue[2] = offAtMinute;
  offAtValue[3] = offAtSecond;
  offAtChar.setValue(offAtValue, 4);
  offAtChar.setCallback(offAtCallback);

//  current RGB char setup
  byte rgbDefault[3] = {defaultColor[0], defaultColor[1], defaultColor[2]};
  rgbDefaultChar.setValue(rgbDefault, 3);
  rgbDefaultChar.setCallback(rgbDefaultCallback);


  //adding services
  myService.addCharacteristic(rgbDefaultChar);
  myService.addCharacteristic(offAtChar);
  myService.addCharacteristic(onAtChar);
  myService.addCharacteristic(timeChar);
  myService.addCharacteristic(alarmOffTimeChar);
  myService.addCharacteristic(alarmOffChar);
  myService.addCharacteristic(alarmOnTimeChar);
  myService.addCharacteristic(alarmOnChar);
  myService.addCharacteristic(rgbChar);
  myService.addCharacteristic(lightStatusChar);
  //add the Service
  DuoBLE.addService(myService);


  // Start stack
  DuoBLE.begin();
  // The Advertised name and "local" name should have some agreement
  DuoBLE.advertisingDataAddName(ADVERTISEMENT, deviceName);
  DuoBLE.setName(deviceName);

  // Start advertising.
  DuoBLE.startAdvertising();
}



void loop() {

}



