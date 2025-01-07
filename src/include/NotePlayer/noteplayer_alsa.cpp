#include "noteplayer_alsa.h"
#include <cmath>
#include <stdexcept>
#include <unistd.h>

void NotePlayerAlsa::play(const std::string &note, int octave,
                          const std::string &value, SoundPlayer &player,
                          const int bpm) {
  int duration = getDuration(value, bpm);
  if (!notes_.contains(note))
    throw std::invalid_argument("Invalid note: " + note);
  player.playTone(notes_.at(note) * std::pow(2, octave), duration);
}