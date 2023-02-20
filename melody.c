#define _WIN32_WINNT 0x0A00
#include <windows.h>

#include "melody.h"


// Имперский марш
void Melody(){
    int freq[39] =   {392, 392, 392, 311, 466, 392, 311, 466, 392,
                      587, 587, 587, 622, 466, 369, 311, 466, 392,
                      784, 392, 392, 784, 739, 698, 659, 622, 659,
                      415, 554, 523, 493, 466, 440, 466,
                      311, 369, 311, 466, 392
                      }; // частота нот

    int durations[39] =  {350, 350, 350, 250, 100, 350, 250, 100, 700,
                      350, 350, 350, 250, 100, 350, 250, 100, 700,
                      350, 250, 100, 350, 250, 100, 100, 100, 450,
                      150, 350, 250, 100, 100, 100, 450,
                      150, 350, 250, 100, 750
                      };  // длительность нот

    // Имперский марш
    for (int thisNote = 0; thisNote < 39; thisNote++)
        Beep(freq[thisNote], durations[thisNote]);
}


// Мелодия GameOver
void MelodyGameOver(){
    Beep(NOTE_DS5, 50);
    Beep(NOTE_D5, 50);
    Beep(NOTE_CS5, 50);
    Beep(NOTE_C5 , 150);
}


// Мелодия LevelUp
void MelodyLevelUp(){
    Beep(NOTE_E4, 50);
    Beep(NOTE_G4, 50);
    Beep(NOTE_E5, 50);
    Beep(NOTE_C5, 50);
    Beep(NOTE_D5, 50);
    Beep(NOTE_G5, 50);
}

