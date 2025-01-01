#pragma once

#include "../Speaker/speaker.h"
#include <map>
#include <string>
#include <unordered_map>
class NotePlayer {
public:
  NotePlayer();
  int getFractionary(const std::string &valueName) const;
  int getDuration(const std::string &valueName, const int bpm) const;
  void play(const std::string &note, int octave, const std::string &value,
            Speaker &speaker, const int bpm);

protected:
  // bpm to ms duration is generally calculated based on the quarter note duration
  // in order to generalize we calculate the quadruple of that and then adapt it to
  // the duration in accordance to their fractionary value
  static constexpr int TIME_MS_QUAD = 240000;
  std::unordered_map<std::string, int> durations_;
  std::unordered_map<std::string, float> notes_;
};