#include "NcursesDrawer.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

// Define the drawing characters (same #defines as before)
#define CHAR_NOTE_STEM_STD   '|'
#define CHAR_NOTE_STEM_FRC   '\\'
#define CHAR_NOTE_HEAD_BLANK 'O'
#define CHAR_NOTE_HEAD_FULL  '0'
#define CHAR_NOTE_HEAD_S     '#'
#define CHAR_NOTE_HEAD_F     'b'
#define CHAR_STAFF           '_'

NcursesDrawer::NcursesDrawer()
    : m_notePositionX(1) // Start notes from column 1
{
}

NcursesDrawer::~NcursesDrawer() {
  end(); // Just in case user forgets
}

void NcursesDrawer::init() {
  initscr();
  noecho();
  curs_set(FALSE);
  nodelay(stdscr, TRUE); // Non-blocking input
}

void NcursesDrawer::end() {
  endwin(); // Clean up ncurses
}

void NcursesDrawer::drawStaff(int middleMIDINote) {
  (void) middleMIDINote; // We don't strictly need it to draw lines.
  clear();

  int staffStartCol = 0;
  int staffEndCol = COLS - 1;
  int middleY = LINES / 2;

  // Draw 5 horizontal staff lines
  for (int i = 0; i < 5; ++i) {
    int lineY = middleY - 4 + (i * 2);
    for (int x = staffStartCol; x < staffEndCol; ++x) {
      mvaddch(lineY, x, CHAR_STAFF);
    }
  }
  refresh();
}

void NcursesDrawer::drawLedgerLines(int verticalPosition, int notePositionX, int middleY) {
  // The top and bottom staff lines
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
}

void NcursesDrawer::drawNote(const std::string &note,
                             int octave,
                             const std::string &value,
                             int fractionary,
                             int fractionaryStemCount,
                             int middleMIDINote,
                             int midiNoteNumber,
                             int counter) {
  int middleY = LINES / 2;
  // Vertical position relative to the middle note
  int verticalPosition = middleY - (midiNoteNumber - middleMIDINote);

  // Display info at the top
  mvprintw(0, 0, "Playing: %s%d %s (#%d)        ",
           note.c_str(), octave, value.c_str(), counter);

  // Draw ledger lines if needed
  drawLedgerLines(verticalPosition, m_notePositionX, middleY);

  // Draw the note stem if fractionary indicates a shorter note
  if (fractionary > 1) {
    // Decide stem direction (if note is above or below middle lines)
    if (verticalPosition < middleY) {
      // Upward stem
      for (int y = verticalPosition + 4; y >= verticalPosition + 1; --y) {
        mvaddch(y, m_notePositionX + 1, CHAR_NOTE_STEM_STD);
        // Add slash beams if fractionaryStemCount > 0
        if (fractionaryStemCount-- > 0) {
          mvaddch(y, m_notePositionX, CHAR_NOTE_STEM_FRC);
        }
      }
    } else {
      // Downward stem
      for (int y = verticalPosition - 4; y <= verticalPosition - 1; ++y) {
        mvaddch(y, m_notePositionX, CHAR_NOTE_STEM_STD);
        if (fractionaryStemCount-- > 0) {
          mvaddch(y, m_notePositionX + 1, CHAR_NOTE_STEM_FRC);
        }
      }
    }
  }

  // Draw the note head
  if (fractionary > 2)
    mvaddch(verticalPosition, m_notePositionX - 1, CHAR_NOTE_HEAD_FULL);
  else
    mvaddch(verticalPosition, m_notePositionX - 1, CHAR_NOTE_HEAD_BLANK);

  // Draw sharp (#) or flat (b) if needed
  if (note.find("#") != std::string::npos) {
    mvaddch(verticalPosition, m_notePositionX + 1, CHAR_NOTE_HEAD_S);
  } else if (note.find("b") != std::string::npos) {
    mvaddch(verticalPosition, m_notePositionX + 1, CHAR_NOTE_HEAD_F);
  }

  // Move the horizontal position for the next note
  m_notePositionX += 5;
  // If we run out of space, reset and redraw staff
  if (m_notePositionX >= COLS - 5) {
    m_notePositionX = 1;
    drawStaff(middleMIDINote);
  }

  refresh();
}

void NcursesDrawer::displayIdle() {
  mvprintw(0, 0, "Idle");
  wclrtoeol(stdscr);
  refresh();
}

void NcursesDrawer::waitForExit() {
  mvprintw(LINES - 1, 0, "Press any key to exit...");
  refresh();
  nodelay(stdscr, FALSE); // Switch to blocking for final getch
  getch();
}
