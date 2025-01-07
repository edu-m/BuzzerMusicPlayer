#ifndef NCURSES_DRAWER_H
#define NCURSES_DRAWER_H

#include <stdexcept>
#include <string>
#include <ncurses.h>
#include <unordered_map>

inline int getNoteOffset(const std::string &note) {
  static std::unordered_map<std::string, int> noteOffsets = {
      {"C", 0},  {"C#", 1}, {"Db", 1},  {"D", 2},   {"D#", 3}, {"Eb", 3},
      {"E", 4},  {"F", 5},  {"F#", 6},  {"Gb", 6},  {"G", 7},  {"G#", 8},
      {"Ab", 8}, {"A", 9},  {"A#", 10}, {"Bb", 10}, {"B", 11}};

  if (!noteOffsets.contains(note))
    throw std::invalid_argument("Invalid note name: " + note);
  return noteOffsets.at(note);
}


class NcursesDrawer {
public:
  NcursesDrawer();
  ~NcursesDrawer();

  // Must be called once before using ncurses
  void init();
  // Must be called once when you no longer need ncurses
  void end();

  // Clears the screen and draws the staff lines based on a 'middle' MIDI note
  void drawStaff(int middleMIDINote);

  // Draw a note on the staff
  //  - note: e.g. "C", "C#", "Db", ...
  //  - octave: numeric octave index
  //  - value: rhythmic value string (e.g., "4", "8", "16", "8d", etc.)
  //  - fractionary: for controlling the note-head & stem logic
  //  - fractionaryStemCount: how many "slash" beams you want on the stem
  //  - middleMIDINote: the 'center line' of the staff
  //  - midiNoteNumber: the note’s MIDI number
  //  - counter: how many notes have been played so far (used for display)
  void drawNote(const std::string &note,
                int octave,
                const std::string &value,
                int fractionary,
                int fractionaryStemCount,
                int middleMIDINote,
                int midiNoteNumber,
                int counter);

  // Clears the area at the top, can be used to show the user is "Idle"
  void displayIdle();

  // Wait for user input to exit (blocking)
  void waitForExit();

private:
  void drawLedgerLines(int verticalPosition, int notePositionX, int middleY);

private:
  // Tracks the x-position where the next note will be drawn
  int m_notePositionX;
};

#endif // NCURSES_DRAWER_H
