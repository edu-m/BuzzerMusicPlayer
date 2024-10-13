#pragma once

#include "noteplayer.h"
#include "../SoundPlayer/soundplayer.h"

class NotePlayerAlsa : protected NotePlayer {
public:
using NotePlayer::NotePlayer;
  int getDuration(const std::string &valueName, const int bpm) const;
  void play(const std::string &note, int octave, const std::string &value,
            SoundPlayer &player, const int bpm);
};
