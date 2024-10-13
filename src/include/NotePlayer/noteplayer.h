#pragma once

#include <string>
#include <map>
#include "../Speaker/speaker.h"
class NotePlayer {
public:
    NotePlayer();
    int getDuration(const std::string &valueName, const int bpm) const;
    void play(const std::string& note, int octave, const std::string& value, Speaker& speaker, const int bpm);

protected:
    const int TIME_MS_QUAD = 240000;
    std::map<std::string, int> durations_;
    std::map<std::string, float> notes_;
};