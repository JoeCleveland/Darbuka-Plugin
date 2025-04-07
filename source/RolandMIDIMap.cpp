#include "RolandMIDIMap.h"
#include <iostream>

double RolandMIDIMap::getLocation(int MIDI_Note) {
    std::cout << "MIDI " << MIDI_Note << std::endl;
    if(MIDI_Note == 60) { //CLOSE LEFT
        return 0.3;
    } else if(MIDI_Note == 61) { //CLOSE RIGHT
        return 0.0;
    } else if(MIDI_Note == 62) { //BACK LEFT
        return 2.0;
    } else if(MIDI_Note == 63) { // BACK RIGHT
        return 2.8;
    } else if(MIDI_Note == 64) { //CENTER
        return 0.6;
    } else {
        return (MIDI_Note - 65) * 0.45;
    }
}

int RolandMIDIMap::getPressingLocation(int MIDI_Note) {
    std::cout << "MIDI " << MIDI_Note << std::endl;
    if(MIDI_Note == 60) { //CLOSE LEFT
        return 10;
    } else if(MIDI_Note == 61) { //CLOSE RIGHT
        return 20;
    } else if(MIDI_Note == 62) { //BACK LEFT
        return 50;
    } else if(MIDI_Note == 63) { // BACK RIGHT
        return 60;
    } else if(MIDI_Note == 64) { //CENTER
        return 0;
    } else {
        return 0;
    }
}