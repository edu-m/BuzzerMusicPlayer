#include "include/NcursesDrawer/NcursesDrawer.h"
#include "include/NotePlayer/noteplayer.h"
#include "include/Speaker/speaker.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <curses.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

int getNoteOffset(const std::string &note);

class NcursesSession {
public:
  NcursesSession() {
    initscr();
    cbreak(); // disable line buffering
    noecho(); // don’t echo input
    keypad(stdscr, true);
    curs_set(0); // hide cursor if desired
  }
  NcursesSession(const NcursesSession &) = delete;
  NcursesSession &operator=(const NcursesSession &) = delete;
  NcursesSession(NcursesSession &&) = delete;
  NcursesSession &operator=(NcursesSession &&) = delete;

  ~NcursesSession() { endwin(); }
};

namespace {
std::weak_ptr<Speaker> g_speakerWeak;
extern "C" void handleSignal(int signum) {
  if (signum == SIGINT) {
    if (auto spk = g_speakerWeak.lock()) {
      spk->stop();
    }
    endwin();
    std::exit(signum);
  }
}
}
void printUsage(const char *progName) {
  std::cerr << "Usage: " << progName << " <file_name>\n";
}
int main(int argc, char **argv) try {
  if (argc < 2) {
    printUsage(argv[0]);
    return EXIT_FAILURE;
  }
  const std::string inputFileName = argv[1];
  std::signal(SIGINT, handleSignal);
  auto speaker = std::make_shared<Speaker>();
  g_speakerWeak = speaker;
  NotePlayer notePlayer; // Adjust constructor logic if needed
  std::ifstream input{inputFileName};
  if (!input.is_open()) {
    std::cerr << "Failed to open input file: " << inputFileName << "\n";
    return EXIT_FAILURE;
  }
  NcursesSession ncursesSession;
  NcursesDrawer drawer;
  drawer.init();
  int middleMIDINote = 60; // Middle C
  drawer.drawStaff(middleMIDINote);
  std::string note;
  std::string value;
  int octave = 0;
  int bpm = 100;
  int noteCounter = 0;
  while (true) {
    if (!(input >> note)) {
      break;
    }

    if (note == "bpm") {
      if (!(input >> bpm)) {
        std::cerr << "Error: expected BPM value after 'bpm' command.\n";
        break;
      }
    } else if (note == "P") {
      if (!(input >> value)) {
        std::cerr << "Error: expected duration value after 'P' command.\n";
        break;
      }
      int durationMs = notePlayer.getDuration(value, bpm);
      std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    } else {
      if (!(input >> octave >> value)) {
        std::cerr
            << "Malformed note entry: expected <note> <octave> <value>.\n";
        break;
      }
      notePlayer.play(note, octave, value, *speaker, bpm);
      int noteOffset = getNoteOffset(note);
      int midiNoteNumber = (octave + 1) * 12 + noteOffset;
      {
        int middleY = LINES / 2;
        int verticalPosition = middleY - (midiNoteNumber - middleMIDINote);
        if (verticalPosition < 2 || verticalPosition > (LINES - 2)) {
          middleMIDINote = midiNoteNumber;
          drawer.drawStaff(middleMIDINote);
        }
      }
      int fractionary = notePlayer.getFractionary(value);
      int fractionaryStemCount =
          std::max(0, static_cast<int>(std::log2(fractionary) - 2));

      ++noteCounter;
      drawer.drawNote(note, octave, value, fractionary, fractionaryStemCount,
                      middleMIDINote, midiNoteNumber, noteCounter);
    }
    int ch = getch();
    if (ch == 'q' || ch == 'Q') {
      break;
    }
  }
  drawer.displayIdle();
  drawer.waitForExit();
  drawer.end();

  return EXIT_SUCCESS;
} catch (const std::exception &e) {
  std::cerr << "Exception: " << e.what() << '\n';
  endwin(); // Fallback in case the RAII type didn’t handle something
  return EXIT_FAILURE;
} catch (...) {
  std::cerr << "Unknown error occurred.\n";
  endwin();
  return EXIT_FAILURE;
}
