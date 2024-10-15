#pragma once

#include "../SoundPlayer/soundplayer.h"
#include "noteplayer.h"

class NotePlayerAlsa : protected NotePlayer {
public:
  using NotePlayer::NotePlayer;
  using NotePlayer::getDuration;
  void play(const std::string &note, int octave, const std::string &value,
            SoundPlayer &player, const int bpm);
};
