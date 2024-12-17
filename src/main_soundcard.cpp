#include "include/NotePlayer/noteplayer_alsa.h"
#include "include/SoundPlayer/soundplayer.h"
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <ncurses.h>
#include <portaudio.h>
#include <string>
#include <unistd.h>

#define CHAR_NOTE_STEM_STD '|'
#define CHAR_NOTE_STEM_FRC '\\'
#define CHAR_NOTE_HEAD_BLANK 'O'
#define CHAR_NOTE_HEAD_FULL '0'
#define CHAR_NOTE_HEAD_S '#'
#define CHAR_NOTE_HEAD_F 'b'
#define CHAR_STAFF '_'

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

// Function to draw the staff lines based on the middle MIDI note
void drawStaff() {
  clear();
  int staffStartCol = 0;
  int staffEndCol = COLS - 1;
  int middleY = LINES / 2;
  // Positions of the staff lines relative to the middle note
  int staffLineOffsets[] = {-4, -2, 0, 2, 4}; // Positions for the 5 staff lines
  for (int i = 0; i < 5; ++i) {
    int y = middleY + staffLineOffsets[i];
    for (int x = staffStartCol; x < staffEndCol; ++x) {
      mvaddch(y, x, CHAR_STAFF);
    }
  }
}

int main(int argc, char **argv) {
  int counter = 0;
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

    // Initialize ncurses
    initscr();
    noecho();
    curs_set(FALSE);
    nodelay(stdscr, TRUE); // Non-blocking input

    // Set initial middle note for the staff (e.g., middle C)
    int middleMIDINote = 60; // MIDI number for middle C (C4)
    drawStaff();
    refresh();

    std::string note, value;
    int bpm = 100;
    int octave;
    int notePositionX = 1; // Horizontal position of the note

    while (input >> note) {
      if (note == "bpm") {
        input >> bpm;
      } else if (note == "P") {
        input >> value;
        int duration = notePlayer.getDuration(value, bpm);

        // Display pause message
        // mvprintw(0, 0, "Pause for %s    ", value.c_str());
        refresh();

        usleep(duration * 1000);
      } else {
        input >> octave >> value;

        // Play the note
        notePlayer.play(note, octave, value, player, bpm);
        int duration = notePlayer.getDuration(value, bpm);
        // Calculate MIDI note number
        int noteOffset = getNoteOffset(note);
        int midiNoteNumber = (octave + 1) * 12 + noteOffset;

        // Check if the note is within the displayable range
        int middleY = LINES / 2;
        int verticalPosition = middleY - (midiNoteNumber - middleMIDINote);

        if (verticalPosition < 2 || verticalPosition > LINES - 2) {
          // Adjust the middle note to bring the note into view
          middleMIDINote = midiNoteNumber;
          drawStaff();
          verticalPosition = middleY - (midiNoteNumber - middleMIDINote);
        }

        // Display the note being played
        mvprintw(0, 0, "Playing: %s%d %s (%d ms) #%d       ", note.c_str(),
                 octave, value.c_str(), duration, ++counter);
        refresh();

        // Draw ledger lines if necessary
        int staffLineOffsets[] = {-4, -2, 0, 2, 4};
        int topStaffLine = middleY + staffLineOffsets[0];
        int bottomStaffLine = middleY + staffLineOffsets[4];

        if (verticalPosition > bottomStaffLine) {
          // Below the staff
          for (int i = bottomStaffLine + 2; i <= verticalPosition; i += 2) {
            for (int x = notePositionX - 2; x <= notePositionX + 2; ++x) {
              mvaddch(i, x, CHAR_STAFF);
            }
          }
        } else if (verticalPosition < topStaffLine) {
          // Above the staff
          for (int i = topStaffLine - 2; i >= verticalPosition; i -= 2) {
            for (int x = notePositionX - 2; x <= notePositionX + 2; ++x) {
              mvaddch(i, x, CHAR_STAFF);
            }
          }
        }
        int fractionary = notePlayer.getFractionary(value);
        int fractionary_stem_count = log2(fractionary) - 2;

        // Draw the note stem
        if (fractionary > 1) {
          if (verticalPosition < middleY) {
            // Upward stem
            for (int y = verticalPosition + 4; y >= verticalPosition + 1; --y) {
              mvaddch(y, notePositionX + 1, CHAR_NOTE_STEM_STD);
              if (fractionary_stem_count-- > 0)
                mvaddch(y, notePositionX, CHAR_NOTE_STEM_FRC);
            }
          } else {
            // Downward stem
            for (int y = verticalPosition - 4; y <= verticalPosition - 1; ++y) {
              mvaddch(y, notePositionX, CHAR_NOTE_STEM_STD);
              if (fractionary_stem_count-- > 0)
                mvaddch(y, notePositionX + 1, CHAR_NOTE_STEM_FRC);
            }
          }
        }

        // Draw the note head
        if (fractionary > 2)
          mvaddch(verticalPosition, notePositionX - 1, CHAR_NOTE_HEAD_FULL);
        else
          mvaddch(verticalPosition, notePositionX - 1, CHAR_NOTE_HEAD_BLANK);

        if (note.find("#") != std::string::npos)
          mvaddch(verticalPosition, notePositionX + 1, CHAR_NOTE_HEAD_S);
        else if (note.find("b") != std::string::npos)
          mvaddch(verticalPosition, notePositionX + 1, CHAR_NOTE_HEAD_F);

        // Move to next horizontal position
        notePositionX += 5;
        if (notePositionX >= COLS - 5) {
          notePositionX = 1;
          drawStaff();
          refresh();
        }

        // Check for user input to exit
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
          break;
        }
      }
    }

    mvprintw(0, 0, "Idle");
    wclrtoeol(stdscr);
    // Wait for user input before exiting
    mvprintw(LINES - 1, 0, "Press any key to exit...");
    refresh();
    nodelay(stdscr, FALSE); // Blocking input for the final getch()
    getch();

    // Clean up ncurses
    endwin();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    endwin(); // Clean up ncurses in case of exception
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
