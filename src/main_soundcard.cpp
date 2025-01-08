#include "include/NcursesDrawer/NcursesDrawer.h"
#include "include/NotePlayer/noteplayer_soundcard.h"
#include "include/SoundPlayer/soundplayer.h"

#include <chrono> // for std::chrono::milliseconds
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <curses.h> // for LINES, getch()
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <portaudio.h>
#include <string>
#include <thread> // for std::this_thread::sleep_for
int getNoteOffset(const std::string &note);
class NcursesSession {
public:
  NcursesSession() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, true);
    curs_set(0); // Hide the cursor if desired
  }
  NcursesSession(const NcursesSession &) = delete;
  NcursesSession &operator=(const NcursesSession &) = delete;
  NcursesSession(NcursesSession &&) = delete;
  NcursesSession &operator=(NcursesSession &&) = delete;

  ~NcursesSession() { endwin(); }
};
class PortAudioSession {
public:
  PortAudioSession() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
      throw std::runtime_error(std::string("PortAudio init failed: ") +
                               Pa_GetErrorText(err));
    }
  }
  PortAudioSession(const PortAudioSession &) = delete;
  PortAudioSession &operator=(const PortAudioSession &) = delete;
  PortAudioSession(PortAudioSession &&) = delete;
  PortAudioSession &operator=(PortAudioSession &&) = delete;

  ~PortAudioSession() {
    PaError err = Pa_Terminate();
    if (err != paNoError) {
      std::cerr << "Warning: PortAudio terminate failed: "
                << Pa_GetErrorText(err) << std::endl;
    }
  }
};
namespace {
std::weak_ptr<PortAudioSession> g_portaudioWeak;
}
extern "C" void handle_signal(int signum) {
  if (signum == SIGINT) {
    if (auto paSession = g_portaudioWeak.lock()) {
    }
    endwin(); // ensure curses is cleaned up
    std::exit(signum);
  }
}
void printUsage(const char *programName) {
  std::cerr << "Usage: " << programName << " <file_name>\n";
}
int main(int argc, char **argv) try {
  if (argc < 2) {
    printUsage(argv[0]);
    return EXIT_FAILURE;
  }
  char selection = 'Q'; // default is square wave
  if (argc >= 3) {
    if (*argv[2] != 'Q' && *argv[2] != 'W' && *argv[2] != 'S' &&
        *argv[2] != 'T')
      std::cout << "Invalid wave selection. Defaulting to square" << std::endl;
    else
      selection = *argv[2];
  }
  const std::string fileName = argv[1];
  std::signal(SIGINT, handle_signal);
  auto portaudioSession = std::make_shared<PortAudioSession>();
  g_portaudioWeak = portaudioSession;
  NcursesSession ncursesSession;
  SoundPlayer player(selection);
  NotePlayerAlsa notePlayer;
  NcursesDrawer drawer;
  drawer.init();
  std::ifstream input(fileName);
  if (!input.is_open()) {
    std::cerr << "Failed to open file: " << fileName << "\n";
    return EXIT_FAILURE;
  }
  int middleMIDINote = 60;
  drawer.drawStaff(middleMIDINote);
  int bpm = 100;
  int octave = 0;
  int noteCounter = 0;
  std::string note;
  std::string value;
  while (true) {
    if (!(input >> note)) {
      break;
    }

    if (note == "bpm") {
      if (!(input >> bpm)) {
        std::cerr << "Error: 'bpm' command requires a numeric value.\n";
        break;
      }
    } else if (note == "P") {
      if (!(input >> value)) {
        std::cerr << "Error: 'P' command requires a duration.\n";
        break;
      }
      int durationMs = notePlayer.getDuration(value, bpm);
      std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
    } else {
      if (!(input >> octave >> value)) {
        std::cerr << "Malformed note line. Expected: <note> <octave> <value>\n";
        break;
      }
      notePlayer.play(note, octave, value, player, bpm);
      int noteOffset = getNoteOffset(note);
      int midiNoteNumber = (octave + 1) * 12 + noteOffset;
      int middleY = LINES / 2;
      int verticalPosition = middleY - (midiNoteNumber - middleMIDINote);
      if (verticalPosition < 2 || verticalPosition > (LINES - 2)) {
        middleMIDINote = midiNoteNumber;
        drawer.drawStaff(middleMIDINote);
      }
      int fractionary = notePlayer.getFractionary(value);
      int fractionaryStemCount =
          std::max(0, static_cast<int>(std::log2(fractionary) - 2));
      drawer.drawNote(note, octave, value, fractionary, fractionaryStemCount,
                      middleMIDINote, midiNoteNumber, ++noteCounter);
    }
    int ch = getch();
    if (ch == 'q' || ch == 'Q')
      break;
  }
  drawer.displayIdle();
  drawer.waitForExit();
  drawer.end();

  return EXIT_SUCCESS;
} catch (const std::exception &e) {
  std::cerr << "Error: " << e.what() << std::endl;
  endwin();
  return EXIT_FAILURE;
} catch (...) {
  std::cerr << "Unknown error occurred.\n";
  endwin();
  return EXIT_FAILURE;
}
