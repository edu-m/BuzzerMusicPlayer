#include "include/NcursesDrawer/NcursesDrawer.h"
#include "include/NotePlayer/noteplayer_alsa.h"
#include "include/SoundPlayer/soundplayer.h"
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <portaudio.h>
#include <string>
#include <unistd.h>

// -------------------------------------------------------------------
// Constants/Defines moved here or remain here
// -------------------------------------------------------------------

void handle_signal(int signum) {
  Pa_Terminate();
  endwin(); // Clean up ncurses before exiting
  exit(signum);
}

// Function to map note names to semitone offsets from C
int getNoteOffset(const std::string &note) {
  static std::map<std::string, int> noteOffsets = {
      {"C", 0},  {"C#", 1}, {"Db", 1},  {"D", 2},   {"D#", 3}, {"Eb", 3},
      {"E", 4},  {"F", 5},  {"F#", 6},  {"Gb", 6},  {"G", 7},  {"G#", 8},
      {"Ab", 8}, {"A", 9},  {"A#", 10}, {"Bb", 10}, {"B", 11}};
  auto it = noteOffsets.find(note);
  if (it != noteOffsets.end()) {
    return it->second;
  } else {
    throw std::invalid_argument("Invalid note name: " + note);
  }
}

int main(int argc, char **argv) {
  int counter = 0; // Just to keep track of number of notes played

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

    // Create and initialize our Ncurses drawer
    NcursesDrawer drawer;
    drawer.init();

    // Set the initial "middle" note for staff drawing
    int middleMIDINote = 60; // Middle C
    drawer.drawStaff(middleMIDINote);

    std::string note, value;
    int bpm = 100;
    int octave;

    while (input >> note) {
      if (note == "bpm") {
        input >> bpm;
      } else if (note == "P") {
        // Pause logic
        input >> value;
        int duration = notePlayer.getDuration(value, bpm);

        // Could display a "pause" message if desired, or just remain idle
        // drawer.displayPause(value); // (if you add such a method)

        usleep(duration * 1000);

      } else {
        // We have a real note: note, octave, value
        input >> octave >> value;

        // Play the note
        notePlayer.play(note, octave, value, player, bpm);

        // Calculate MIDI note number
        int noteOffset = getNoteOffset(note);
        int midiNoteNumber = (octave + 1) * 12 + noteOffset;

        // If the note is out of visible range, shift the staff
        // (You could implement additional logic to auto-scroll)
        int middleY = LINES / 2;
        int verticalPosition = middleY - (midiNoteNumber - middleMIDINote);
        if (verticalPosition < 2 || verticalPosition > LINES - 2) {
          middleMIDINote = midiNoteNumber;
          drawer.drawStaff(middleMIDINote);
        }

        // We'll call the same fractionary logic from notePlayer
        int fractionary = notePlayer.getFractionary(value);
        // e.g. fractionary = 4 -> quarter note, fractionary = 8 -> 8th note,
        // etc. Typically, the number of slash beams is log2(fractionary) - 2 if
        // fractionary >= 8
        int fractionaryStemCount =
            std::max(0, static_cast<int>(std::log2(fractionary) - 2));

        // Ask our drawer to draw the note
        drawer.drawNote(note, octave, value, fractionary, fractionaryStemCount,
                        middleMIDINote, midiNoteNumber, ++counter);

        // Check user input to exit
        int ch = getch();
        if (ch == 'q' || ch == 'Q')
          break;
      }
    }

    // Show "Idle" message at the end, and wait for user input
    drawer.displayIdle();
    drawer.waitForExit();

    // Clean up
    drawer.end(); // or rely on destructor
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    endwin(); // In case something failed, we close ncurses
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
