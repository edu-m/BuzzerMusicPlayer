#include "noteplayer_alsa.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

int NotePlayerAlsa::getDuration(const std::string &valueName,
                                const int bpm) const {
  auto it = durations_.find(valueName);
  if (it != durations_.end()) {
    return TIME_MS_QUAD / (bpm * it->second);
  } else {
    throw std::invalid_argument("Invalid duration value: " + valueName);
  }
}

void NotePlayerAlsa::play(const std::string &note, int octave,
                          const std::string &value, SoundPlayer &player,
                          const int bpm) {
  int duration = getDuration(value, bpm);
  std::cout << note << octave << " (" << duration << " ms)" << std::endl;
  auto it = notes_.find(note);
  if (it != notes_.end()) {
    double frequency = it->second * std::pow(2, octave);
    player.playTone(frequency, duration);
    usleep(duration * 1000); // Rest between notes
  } else {
    throw std::invalid_argument("Invalid note: " + note);
  }
}