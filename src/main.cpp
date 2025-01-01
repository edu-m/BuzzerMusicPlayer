#include "include/NotePlayer/noteplayer.h"
#include "include/Speaker/speaker.h"

// Include the NcursesDrawer
#include "include/NcursesDrawer/NcursesDrawer.h"

#include <cmath>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <unistd.h>

Speaker *g_speaker = nullptr;

int getNoteOffset(const std::string &note) {
  static std::map<std::string, int> noteOffsets = {
      {"C", 0}, {"C#", 1}, {"Db", 1}, {"D", 2}, {"D#", 3}, {"Eb", 3},
      {"E", 4}, {"F", 5},  {"F#", 6}, {"Gb", 6}, {"G", 7}, {"G#", 8},
      {"Ab", 8}, {"A", 9}, {"A#", 10}, {"Bb", 10}, {"B", 11}
  };
  auto it = noteOffsets.find(note);
  if (it != noteOffsets.end()) {
    return it->second;
  } else {
    throw std::invalid_argument("Invalid note name: " + note);
  }
}

// Handle CTRL-C (SIGINT)
void handle_signal(int signum) {
  if (g_speaker != nullptr && signum == SIGINT) {
    g_speaker->stop();
  }
  endwin();
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

    NcursesDrawer drawer;
    drawer.init();
    int middleMIDINote = 60; // e.g., Middle C
    drawer.drawStaff(middleMIDINote);

    std::string note, value;
    int octave;
    int bpm = 100;
    int noteCounter = 0;

    while (input >> note) {
      if (note == "bpm") {
        input >> bpm;
      } else if (note == "P") {
        // Pause
        input >> value;
        int duration = notePlayer.getDuration(value, bpm);
        usleep(duration * 1000);
      } else {
        input >> octave >> value;
        notePlayer.play(note, octave, value, speaker, bpm);

        int noteOffset = getNoteOffset(note);
        int midiNoteNumber = (octave + 1) * 12 + noteOffset;

        // If the note is out of view, shift the staff
        int middleY = LINES / 2;
        int verticalPosition = middleY - (midiNoteNumber - middleMIDINote);

        if (verticalPosition < 2 || verticalPosition > (LINES - 2)) {
          // shift the staff center
          middleMIDINote = midiNoteNumber;
          drawer.drawStaff(middleMIDINote);
        }

        // Use NotePlayer to get fractionary (e.g., 4 = quarter note, 8 = 8th, etc.)
        int fractionary = notePlayer.getFractionary(value);
        // Calculate # of slash beams for 8th, 16th, etc.
        // example: fractionaryStemCount = log2(fractionary) - 2 if fractionary >= 8
        int fractionaryStemCount = 0;
        if (fractionary >= 8) {
          fractionaryStemCount = static_cast<int>(std::log2(fractionary) - 2);
        }

        // Draw the note via NcursesDrawer
        ++noteCounter;
        drawer.drawNote(note, octave, value,
                        fractionary, fractionaryStemCount,
                        middleMIDINote, midiNoteNumber,
                        noteCounter);

        // Check if user pressed 'q' to quit
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
          break;
        }
      }
    }

    // Display "Idle" and wait for user input
    drawer.displayIdle();
    drawer.waitForExit();

    // End ncurses
    drawer.end();
  } 
  catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    endwin(); // Just in case
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
