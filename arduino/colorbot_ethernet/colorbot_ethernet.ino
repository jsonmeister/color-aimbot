#include <Mouse.h>
#include <Ethernet.h>
#include <SPI.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetServer server(31337); 

namespace Stealth {
    uint32_t rngState = 0xDEADBEEF;
    
    uint32_t xorshift32() {
        rngState ^= rngState << 13;
        rngState ^= rngState >> 17;
        rngState ^= rngState << 5;
        return rngState;
    }
    
    uint16_t gaussianDelay() {
        int32_t sum = 0;
        for (int i = 0; i < 4; i++) {
            sum += (xorshift32() & 0xFF);
        }
        return 800 + ((sum - 512) * 200) / 256;
    }
    
    int8_t calculateStep(int8_t remaining) {
        if (remaining == 0) return 0;
        int8_t absVal = abs(remaining);
        int8_t step;
        
        if (absVal > 50) step = 6 + (xorshift32() % 3);
        else if (absVal > 20) step = 4 + (xorshift32() % 3);
        else if (absVal > 8) step = 2 + (xorshift32() % 2);
        else step = 1;
        
        return (remaining > 0) ? min(step, remaining) : max(-step, remaining);
    }
}

volatile int8_t targetX = 0;
volatile int8_t targetY = 0;
uint32_t lastMove = 0;
uint16_t moveDelay = 1000;

void setup() {
    Stealth::rngState = analogRead(A0) ^ (analogRead(A1) << 10);
    if (Stealth::rngState == 0) Stealth::rngState = 0xCAFEBABE;
    
    Ethernet.begin(mac);
    server.begin();
    Mouse.begin();
    
    delay(1000);
}

void loop() {
    uint32_t now = micros();
    
    EthernetClient client = server.available();
    if (client && client.available()) {
        String data = client.readStringUntil('\n');
        int comma = data.indexOf(',');
        
        if (comma != -1) {
            int8_t x = data.substring(0, comma).toInt();
            int8_t y = data.substring(comma + 1).toInt();
            
            targetX += x;
            targetY += y;
            targetX = constrain(targetX, -127, 127);
            targetY = constrain(targetY, -127, 127);
        }
        
        if (data.startsWith("C")) {
            delayMicroseconds(Stealth::gaussianDelay());
            Mouse.click(MOUSE_LEFT);
        }
    }
    
    if (now - lastMove >= moveDelay) {
        if (targetX != 0 || targetY != 0) {
            int8_t stepX = Stealth::calculateStep(targetX);
            int8_t stepY = Stealth::calculateStep(targetY);
            
            // we will add tremor xd
            if ((Stealth::xorshift32() & 0x0F) == 0) {
                stepY += ((Stealth::xorshift32() & 0x01) ? 1 : -1);
            }
            
            Mouse.move(stepX, stepY, 0);
            targetX -= stepX;
            targetY -= stepY;
            
            moveDelay = Stealth::gaussianDelay();
        } else {
            moveDelay = 100;
        }
        lastMove = now;
    }
    
    Ethernet.maintain();
}
