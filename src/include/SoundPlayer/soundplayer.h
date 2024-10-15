#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define SAMPLE_RATE 48000.0 // anything higher should not be necessary
#include <portaudio.h>

class SoundPlayer {
public:
  SoundPlayer();
  ~SoundPlayer();

  void playTone(double frequency, int duration_ms);

private:
  static int paCallback(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags, void *userData);

  PaStream *stream_;

  struct PaData {
    double phase;
    double frequency;
  } data_;
};
