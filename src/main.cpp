#include <RGBmatrixPanel.h>

#define CLK  8
#define OE   9
#define LAT A3
#define A   A0
#define B   A1
#define C   A2

// Last parameter = 'true' enables double-buffering, for flicker-free,
// buttery smooth animation.  Note that NOTHING WILL SHOW ON THE DISPLAY
// until the first call to swapBuffers().  This is normal.
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

// Similar to F(), but for PROGMEM string pointers rather than literals
//#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

int minute = 2;
int second = 0;

String minuteText = "2";
String secondText = "00";

long elapsedTime = 0;

unsigned int pointRed = 0;
unsigned int pointBlue = 0;

bool timerStarted = false;

String timerText[2];

String tempText = "1 37";

String incomingSerial;

void refreshScreen();

void tickTime();
void readSerial();

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
}

void loop() {
    if (elapsedTime + 1000 <= millis() && timerStarted) {
        tickTime();
        refreshScreen();
        elapsedTime = millis();
    }
    readSerial();
}

void tickTime() {
    second -= 1;
    if (second < 0) {
        second = 59;
        minute -= 1;
        if (minute < 0)
            minute = 2;
    }
    integerToString(minute, second);
}

void refreshScreen() {
    matrix.fillScreen(0);
    matrix.setTextColor(matrix.Color333(255, 255, 0));
    matrix.setCursor(6, 0);
    matrix.print(timerText[0]);
    matrix.drawRect(12,1,2,2,matrix.Color333(255,255,0));
    matrix.drawRect(12,4,2,2,matrix.Color333(255,255,0));
//    matrix.setCursor(10, 0);
//    matrix.print(":");
    matrix.setCursor(15, 0);
    matrix.print(timerText[1]);


    matrix.setTextColor(matrix.Color333(255, 0, 0));
    matrix.setCursor(8, 8);
    matrix.print((String) pointRed);

    matrix.fillRect(14, 11, 4, 1, matrix.Color333(70, 70, 70));

    matrix.setTextColor(matrix.Color333(0, 0, 255));
    matrix.setCursor(19, 8);
    matrix.print((String) pointBlue);
}

void readSerial() {
    if (Serial.available() > 0) {
        incomingSerial = Serial.readString();
        switch (incomingSerial[0]) {
            case 'R': {
                pointRed += 1;
                refreshScreen();
                break;
            }
            case 'B': {
                pointBlue += 1;
                refreshScreen();
                break;
            }
            case 'S': {
                timerStarted = true;
                break;
            }
            case 'P': {
                timerStarted = false;
                break;
            }
            case 'Z': {
                timerStarted = false;
                minute = 2;
                second = 00;
                integerToString(minute, second);
                pointBlue = 0;
                pointRed = 0;
                refreshScreen();
                break;
            }
        }
    }
}