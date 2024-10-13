#include "include/NotePlayer/noteplayer.h"
#include "include/Speaker/speaker.h"
#include <cmath>
#include <csignal>
#include <fstream>
#include <iostream>

Speaker *g_speaker = nullptr;

void handle_signal(int signum) {
  if (g_speaker != nullptr && signum == SIGINT) {
    g_speaker->stop();
  }
  exit(signum);
}

int main(int argc, char **argv) {
  try {
    std::signal(SIGINT, handle_signal);
    Speaker speaker;
    g_speaker = &speaker;
    NotePlayer notePlayer;

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
    int octave;
    int bpm = 100;
    while (input >> note) {
      if (note == "bpm") {
        input >> bpm;
      } else if (note == "P") {
        input >> value;
        int duration = notePlayer.getDuration(value, bpm);
        usleep(1000 * duration);
      } else {
        input >> octave >> value;
        notePlayer.play(note, octave, value, speaker, bpm);
      }
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return 0;
}
