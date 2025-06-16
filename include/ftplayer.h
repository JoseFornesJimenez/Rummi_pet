#ifndef MY_FTPLAYER_H
#define MY_FTPLAYER_H

#include <DFRobotDFPlayerMini.h>

class FTPlayer {
public:
    FTPlayer() : myDFPlayer() {}
    // Devuelve true si la inicialización fue exitosa
    bool begin(HardwareSerial &serial) {
        serial.begin(9600);
        return myDFPlayer.begin(serial);
    }
    void setVolume(uint8_t vol) {
        myDFPlayer.volume(vol);
    }
    void playSong(uint8_t num) {
        myDFPlayer.play(num);
    }
    // Devuelve true si el DFPlayer está parado
    bool isStopped() {
        myDFPlayer.available(); // Procesa el buffer serie
        return myDFPlayer.readState() == 0;
    }
private:
    DFRobotDFPlayerMini myDFPlayer;
};

#endif // MY_FTPLAYER_H
