#include "NcursesDrawer.h"

#include <cmath>
static constexpr char CHAR_NOTE_STEM_STD = '|';
static constexpr char CHAR_NOTE_STEM_FRC = '\\';
static constexpr char CHAR_NOTE_HEAD_BLANK = 'O';
static constexpr char CHAR_NOTE_HEAD_FULL = '0';
static constexpr char CHAR_NOTE_HEAD_S = '#';
static constexpr char CHAR_NOTE_HEAD_F = 'b';
static constexpr char CHAR_STAFF = '_';
NcursesDrawer::NcursesDrawer()
    : m_notePositionX(1) // Start notes from column 1
{}

NcursesDrawer::~NcursesDrawer() { end(); }

void NcursesDrawer::init() {
  initscr();
  noecho();
  curs_set(FALSE);
  nodelay(stdscr, TRUE);
}

void NcursesDrawer::end() {
  if (!isendwin())
    endwin();
}

void NcursesDrawer::drawStaff(int middleMIDINote) {
  clear();

  const int staffStartCol = 0;
  const int staffEndCol = COLS - 1;
  const int middleY = LINES / 2;
  for (int i = 0; i < 5; ++i) {
    const int lineY = middleY - 4 + (i * 2);
    for (int x = staffStartCol; x < staffEndCol; ++x)
      mvaddch(lineY, x, CHAR_STAFF);
  }
  refresh();
}

void NcursesDrawer::drawLedgerLines(int verticalPosition, int notePositionX,
                                    int middleY) {
  static constexpr int staffLineOffsets[] = {-4, -2, 0, 2, 4};
  const int topStaffLine = middleY + staffLineOffsets[0];
  const int bottomStaffLine = middleY + staffLineOffsets[4];
  if (verticalPosition > bottomStaffLine) {
    for (int y = bottomStaffLine + 2; y <= verticalPosition; y += 2)
      for (int x = notePositionX - 2; x <= notePositionX + 2; ++x)
        mvaddch(y, x, CHAR_STAFF);
  } else if (verticalPosition < topStaffLine) {
    for (int y = topStaffLine - 2; y >= verticalPosition; y -= 2)
      for (int x = notePositionX - 2; x <= notePositionX + 2; ++x)
        mvaddch(y, x, CHAR_STAFF);
  }
}

void NcursesDrawer::drawNote(const std::string &note, int octave,
                             const std::string &value, int fractionary,
                             int fractionaryStemCount, int middleMIDINote,
                             int midiNoteNumber, int counter) {
  const int middleY = LINES / 2;
  const int verticalPosition = middleY - (midiNoteNumber - middleMIDINote);
  mvprintw(0, 0, "Playing: %s%d %s (#%d)        ", note.c_str(), octave,
           value.c_str(), counter);
  drawLedgerLines(verticalPosition, m_notePositionX, middleY);
  if (fractionary > 1) {
    if (verticalPosition < middleY) {
      for (int y = verticalPosition + 4; y >= verticalPosition + 1; --y) {
        mvaddch(y, m_notePositionX + 1, CHAR_NOTE_STEM_STD);
        if (fractionaryStemCount-- > 0)
          mvaddch(y, m_notePositionX, CHAR_NOTE_STEM_FRC);
      }
    } else {
      for (int y = verticalPosition - 4; y <= verticalPosition - 1; ++y) {
        mvaddch(y, m_notePositionX, CHAR_NOTE_STEM_STD);
        if (fractionaryStemCount-- > 0)
          mvaddch(y, m_notePositionX + 1, CHAR_NOTE_STEM_FRC);
      }
    }
  }
  if (fractionary > 2)
    mvaddch(verticalPosition, m_notePositionX - 1, CHAR_NOTE_HEAD_FULL);
  else
    mvaddch(verticalPosition, m_notePositionX - 1, CHAR_NOTE_HEAD_BLANK);

  if (note.find('#') != std::string::npos)
    mvaddch(verticalPosition, m_notePositionX + 1, CHAR_NOTE_HEAD_S);
  else if (note.find('b') != std::string::npos)
    mvaddch(verticalPosition, m_notePositionX + 1, CHAR_NOTE_HEAD_F);

  m_notePositionX += 5;
  if (m_notePositionX >= COLS - 5) {
    m_notePositionX = 1;
    drawStaff(middleMIDINote);
  }

  refresh();
}

void NcursesDrawer::displayIdle() {
  mvprintw(0, 0, "Idle   ");
  wclrtoeol(stdscr); // Clear the rest of the line
  refresh();
}

void NcursesDrawer::waitForExit() {
  mvprintw(LINES - 1, 0, "Press any key to exit...");
  refresh();
  nodelay(stdscr, FALSE);
  getch();
  nodelay(stdscr, TRUE);
}
