#pragma once

#include "../Speaker/speaker.h"
#include <map>
#include <string>
class NotePlayer {
public:
  NotePlayer();
  int getDuration(const std::string &valueName, const int bpm) const;
  void play(const std::string &note, int octave, const std::string &value,
            Speaker &speaker, const int bpm);

protected:
  static constexpr int TIME_MS_QUAD = 240000;
  std::map<std::string, int> durations_;
  std::map<std::string, float> notes_;
};