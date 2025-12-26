#include "HID-Project.h"

// most stupid rng i've ever seen but works
namespace {
    uint32_t rng = 0xDEADBEEF;
    
    uint32_t random32() {
        rng ^= rng << 13;
        rng ^= rng >> 17;
        rng ^= rng << 5;
        return rng;
    }
    
    int16_t randomRange(int16_t min, int16_t max) {
        return min + (random32() % (max - min + 1));
    }
}

int8_t targetX = 0, targetY = 0;
uint32_t lastMove = 0;
uint8_t rawhidData[64];

void setup() {
    rng ^= analogRead(A0) | (analogRead(A1) << 10);
    Mouse.begin();
    RawHID.begin(rawhidData, sizeof(rawhidData));
    pinMode(LED_BUILTIN, OUTPUT);
    delay(100);
}

void loop() {
    int avail = RawHID.available();
    if (avail > 0) {
        digitalWrite(LED_BUILTIN, HIGH);
        
        int8_t x = (int8_t)rawhidData[0];
        int8_t y = (int8_t)rawhidData[1];
        uint8_t cmd = rawhidData[2];
        
        while (RawHID.available()) RawHID.read();
        
        if (cmd == 0x00) {
            targetX = x;
            targetY = y;
        } else if (cmd == 0x01) {
            delayMicroseconds(randomRange(30, 80));
            Mouse.click();
        }
        
        digitalWrite(LED_BUILTIN, LOW);
    }
    
    uint32_t now = micros();
    uint16_t interval = 800 + randomRange(-150, 150);
    
    if (now - lastMove >= interval && (targetX != 0 || targetY != 0)) {
        int8_t absX = abs(targetX);
        int8_t absY = abs(targetY);
        int8_t maxStep = (absX > 30 || absY > 30) ? 10 : 
                         (absX > 15 || absY > 15) ? 6 : 3;
        
        int8_t moveX = constrain(targetX, -maxStep, maxStep);
        int8_t moveY = constrain(targetY, -maxStep, maxStep);
        
        if ((random32() & 0x1F) == 0)
            moveY += (random32() & 1) ? 1 : -1;
        
        Mouse.move(moveX, moveY, 0);
        targetX -= moveX;
        targetY -= moveY;
        lastMove = now;
    }
}
