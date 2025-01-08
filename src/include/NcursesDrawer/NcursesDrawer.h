#pragma once

#include <ncurses.h>
#include <stdexcept>
#include <string>
#include <unordered_map>

inline int getNoteOffset(const std::string &note) {
  static const std::unordered_map<std::string, int> noteOffsets{
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
  void init();
  void end();
  void drawStaff(int middleMIDINote);
  void drawNote(const std::string &note, int octave, const std::string &value,
                int fractionary, int fractionaryStemCount, int middleMIDINote,
                int midiNoteNumber, int counter);
  void displayIdle();
  void waitForExit();

private:
  void drawLedgerLines(int verticalPosition, int notePositionX, int middleY);

private:
  int m_notePositionX;
};
