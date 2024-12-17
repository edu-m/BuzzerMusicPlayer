#include "noteplayer_alsa.h"
#include <cmath>
#include <stdexcept>
#include <unistd.h>

void NotePlayerAlsa::play(const std::string &note, int octave,
                          const std::string &value, SoundPlayer &player,
                          const int bpm) {
  int duration = getDuration(value, bpm);
  // std::cout << note << octave << " (" << duration << " ms)" << std::endl;
  auto it = notes_.find(note);
  if (it != notes_.end()) {
    double frequency = it->second * std::pow(2, octave);
    player.playTone(frequency, duration);
    
  } else {
    throw std::invalid_argument("Invalid note: " + note);
  }
}