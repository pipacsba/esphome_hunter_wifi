#pragma once

#ifndef HunterRoam_h
#define HunterRoam_h

#include <vector>

#define START_INTERVAL 900
#define SHORT_INTERVAL 208
#define LONG_INTERVAL 1875

#define HUNTER_PIN 16 // D0

class HunterRoam {
    public:
        HunterRoam(int pin);
        uint8_t stopZone(uint8_t zone);
        uint8_t startZone(uint8_t zone, uint8_t time);
        uint8_t startProgram(uint8_t num);
        String errorHint(uint8_t error);
    
    private:
        int _pin;
        void hunterBitfield(std::vector <uint8_t> &bits, uint8_t pos, uint8_t val, uint8_t len);
        void writeBus(std::vector<uint8_t> buffer, bool extrabit);
        void sendLow(void);
        void sendHigh(void);
};

#endif
