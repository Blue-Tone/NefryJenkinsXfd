#include "Arduino.h"
#include "pitches.h"
#include "sound.hpp"
#define BEAT 150

// Play OK sounds
void PlayOkSounds(int pin){
  // Marios coin sound
//    tone(pin,NOTE_A5,200);
//    delay(200);
//    tone(pin,NOTE_C6,500);
    
  // Marios 1up sound
  tone(pin,NOTE_E5,BEAT);
  delay(BEAT);
  tone(pin,NOTE_G5,BEAT);
  delay(BEAT);
  tone(pin,NOTE_E5,BEAT);
  delay(BEAT);
  tone(pin,NOTE_C6,BEAT);
  delay(BEAT);
  tone(pin,NOTE_D6,BEAT);
  delay(BEAT);
  tone(pin,NOTE_G6,BEAT);
  delay(BEAT);
  noTone(pin);
}

// Play NG sounds
void PlayNgSounds(int pin){
  tone(pin,NOTE_A4,300);
  delay(500);
  tone(pin,NOTE_A4,700);
  noTone(pin);
}
