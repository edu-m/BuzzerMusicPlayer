#include "noteplayer.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

NotePlayer::NotePlayer() {
  durations_ = {{"w", 1},  {"h", 2},  {"q", 4},  {"e", 8},
                {"s", 16}, {"t", 32}, {"sf", 64}};

  notes_ = {{"C", 16.35},  {"C#", 17.32}, {"Db", 17.32}, {"D", 18.35},
            {"D#", 19.45}, {"Eb", 19.45}, {"E", 20.6},   {"F", 21.83},
            {"F#", 23.12}, {"Gb", 23.12}, {"G", 24.5},   {"G#", 25.96},
            {"Ab", 25.96}, {"A", 27.5},   {"A#", 29.14}, {"Bb", 29.14},
            {"B", 30.87}};
}

int NotePlayer::getDuration(const std::string &valueName, const int bpm) const {
  auto it = durations_.find(valueName);
  if (it != durations_.end()) {
    return TIME_MS_QUAD / (bpm * it->second);
  } else {
    throw std::invalid_argument("Invalid duration value: " + valueName);
  }
}

void NotePlayer::play(const std::string &note, int octave,
                      const std::string &value, Speaker &speaker,
                      const int bpm) {
  int duration = getDuration(value, bpm);
  std::cout << note << octave << " (" << duration << " ms)" << std::endl;
  auto it = notes_.find(note);
  if (it != notes_.end()) {
    float frequency = it->second * std::pow(2, octave);
    speaker.sendTone(static_cast<int>(frequency));
    usleep(1000 * duration);
    speaker.stop();
    usleep(1000 * duration);
  } else {
    throw std::invalid_argument("Invalid note: " + note);
  }
}