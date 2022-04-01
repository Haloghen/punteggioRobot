#include <RGBmatrixPanel.h>

#define CLK  8
#define OE   9
#define LAT A3
#define A   A0
#define B   A1
#define C   A2

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

// Similar to F(), but for PROGMEM string pointers rather than literals
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

int minute = 2;
int second = 0;

String minuteText = "2";
String secondText = "00";

unsigned long elapsedTime = 0;

unsigned int pointRed = 0;
unsigned int pointBlue = 0;

bool timerStarted = false;
bool releaseTimer = false;

unsigned long lastTick = 0;

char timeToRelease = '5';
int remainingTime = 5;

String timerText[2];

int countdown = 3;
String countdownText = "3";
unsigned long lastCountdown = 0;
bool countdownStarted = false;

bool timerPaused = false;

char incomingSerial;

void refreshScreen();

void tickTime();

void readSerial();

void updateTimeToRelease();

void tickCountdown();

const char str[]
PROGMEM = "OFFLINE";
int16_t textX = matrix.width(),
        textMin = (int16_t)
sizeof(str) * -12;

void integerToString(int m, int s) {
    String mText = (String) m;
    String sText = (String) s;
    if (sText.length() < 2) {
        sText = "0" + sText;
    }
    String text[2];
    timerText[0] = mText;
    timerText[1] = sText;
}

void setup() {
    Serial.begin(115200);

    matrix.begin();
    matrix.setTextWrap(false); // Allow text to run off right edge
    matrix.setTextSize(0);

    integerToString(minute, second);
    refreshScreen();

    elapsedTime = millis();
    lastTick = millis();
    lastCountdown = millis();
}

void loop() {
    if (elapsedTime + 1000 <= millis()) {
        if (timerStarted) {
            tickTime();
            refreshScreen();
        } else if (countdownStarted) {
            tickCountdown();
            refreshScreen();
        }
        elapsedTime = millis();
    }
    if (releaseTimer) {
        if (lastTick + 1000 <= millis()) {
            updateTimeToRelease();
            refreshScreen();
            lastTick = millis();
        }
    }

    if (!timerStarted && !countdownStarted && !timerPaused) {
        refreshScreen();
    }

    readSerial();
    delay(20);
}

void updateTimeToRelease() {
    if (remainingTime <= 0) {
        timeToRelease = 'R';
    } else {
        timeToRelease = ((String) remainingTime)[0];
    }
    if (remainingTime == -2) {
        timeToRelease = "";
        releaseTimer = false;
        remainingTime = 5;
    } else remainingTime--;
}

void tickTime() {
    second -= 1;
    if (second < 0) {
        second = 59;
        minute -= 1;
        if (minute < 0) {
            minute = 0;
            second = 0;
            timerStarted = false;
        }
    }
    integerToString(minute, second);
}

void tickCountdown() {
    if (countdown == 0) {
        countdownText = "GO!";
        countdown--;
    } else if (countdown == -1) {
        timerStarted = true;
        countdownStarted = false;
        countdown = 3;
    } else {
        countdownText = ((String) countdown)[0];
        countdown--;
    }
}

void refreshScreen() {
    matrix.fillScreen(0);
    if (timerStarted || timerPaused) {
        matrix.setTextSize(1);
        matrix.setTextColor(matrix.Color333(255, 255, 0));
        matrix.setCursor(6, 0);
        matrix.print(timerText[0]);
        matrix.drawRect(12, 1, 2, 2, matrix.Color333(255, 255, 0));
        matrix.drawRect(12, 4, 2, 2, matrix.Color333(255, 255, 0));

        matrix.setCursor(15, 0);
        matrix.print(timerText[1]);


        if (pointRed < 10)
            matrix.setCursor(8, 8);
        else
            matrix.setCursor(2, 8);

        matrix.setTextColor(matrix.Color333(255, 0, 0));

        matrix.print((String) pointRed);

        matrix.fillRect(14, 11, 4, 1, matrix.Color333(70, 70, 70));

        matrix.setTextColor(matrix.Color333(0, 0, 255));
        matrix.setCursor(19, 8);
        matrix.print((String) pointBlue);

        if (releaseTimer) {
            matrix.setCursor(0, 0);
            matrix.setTextColor(matrix.Color333(255, 255, 255));
            matrix.print(timeToRelease);
        }
    } else if (countdownStarted) {
        if (countdown != -1) {
            matrix.setTextColor(matrix.Color333(255, 0, 0));
            matrix.setCursor(11, 1);
        }
        else {
            matrix.setTextColor(matrix.Color333(0, 255, 0));
            matrix.setCursor(1, 1);
        }
        matrix.print(countdownText);
        matrix.setTextSize(2);
    } else if (!timerPaused) {
        matrix.setTextSize(2);
        matrix.setTextColor(matrix.Color333(255, 255, 0));
        matrix.setCursor(textX, 1);
        matrix.print(F2(str));

        // Move text left (w/wrap), increase hue
        if ((--textX) < textMin) textX = matrix.width();
    }
}

void readSerial() {
    while (Serial.available() > 0) {
//        Serial.println("Reading");
        incomingSerial = Serial.read();
        switch (incomingSerial) {
            case 'R': {
                if (Serial.available() > 0) {
                    incomingSerial = Serial.read();
                    pointRed += incomingSerial - '0';
//                    Serial.println(incomingSerial);
                    refreshScreen();
                }
                break;
            }
            case 'B': {
                if (Serial.available() > 0) {
                    incomingSerial = Serial.read();
                    pointBlue += incomingSerial - '0';
//                    Serial.println(incomingSerial);
                    refreshScreen();
                }
                break;
            }
            case 'S': {
                countdownStarted = true;
                break;
            }
            case 'P': {
                timerStarted = !timerStarted;
                timerPaused = !timerPaused;
                break;
            }
            case 'Z': {
                timerStarted = false;
                timerPaused = false;
                countdownStarted = false;
                minute = 2;
                second = 00;
                integerToString(minute, second);
                pointBlue = 0;
                pointRed = 0;
                refreshScreen();
                break;
            }
            case 'F': {
                if (timerStarted) {
                    releaseTimer = !releaseTimer;
                    if (!releaseTimer)
                        remainingTime = 5;
                    refreshScreen();
                }
                break;
            }
        }
    }
}