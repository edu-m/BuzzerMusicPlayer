#include "include/SoundPlayer/soundplayer.h"
#include "include/NotePlayer/noteplayer_alsa.h"
#include <cmath>
#include <csignal>
#include <fstream>
#include <iostream>
#include <portaudio.h>

void handle_signal(int signum) {
  Pa_Terminate();
  exit(signum);
}

int main(int argc, char **argv) {
  try {
    std::signal(SIGINT, handle_signal);
    SoundPlayer player;
    NotePlayerAlsa notePlayer;

    if (argc < 2) {
      std::cerr << "Usage: speaker <file_name>" << std::endl;
      return EXIT_FAILURE;
    }

    std::ifstream input(argv[1]);
    if (!input) {
      std::cerr << "Failed to open input file: " << argv[1] << std::endl;
      return EXIT_FAILURE;
    }

    std::string note, value;
    int bpm = 100;
    int octave;
    while (input >> note) {
      if (note == "bpm") {
        input >> bpm;
      } else if (note == "P") {
        input >> value;
        int duration = notePlayer.getDuration(value, bpm);
        Pa_Sleep(duration);
      } else {
        input >> octave >> value;
        notePlayer.play(note, octave, value, player, bpm);
      }
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return 0;
}
