#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/kd.h>
#include <map>
#include <math.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define STD_DURATION 1000
#ifndef CLOCK_TICK_RATE
#define CLOCK_TICK_RATE 1193180
#endif
using namespace std;

int dev_console = -1;

map<string, int> durations;
map<string, float> notes;

void beeper_stop() { ioctl(dev_console, KIOCSOUND, 0); }

void handle_signal(int signum) {
  if (dev_console >= 0 && signum == SIGINT) {
    beeper_stop();
    close(dev_console);
  }
  exit(signum);
}

void error(string what) {
  cout << what << endl;
  exit(EXIT_FAILURE);
}

void init() {
  signal(SIGINT, handle_signal);
  if ((dev_console = open("/dev/console", O_WRONLY)) == -1)
    error("open");

  durations["w"] = 1;
  durations["h"] = 2;
  durations["q"] = 4;
  durations["e"] = 8;
  durations["s"] = 16;
  durations["t"] = 32;
  durations["sf"] = 64;

  notes["C"] = 16.35;
  notes["C#"] = 17.32;
  notes["Db"] = 17.32;
  notes["D"] = 18.35;
  notes["D#"] = 19.45;
  notes["Eb"] = 19.45;
  notes["E"] = 20.6;
  notes["F"] = 21.83;
  notes["F#"] = 23.12;
  notes["Gb"] = 23.12;
  notes["G"] = 24.5;
  notes["G#"] = 25.96;
  notes["Ab"] = 25.96;
  notes["A"] = 27.5;
  notes["A#"] = 29.14;
  notes["Bb"] = 29.14;
  notes["B"] = 30.87;
}

void play_beep(int tone, int duration) {
  ioctl(dev_console, KIOCSOUND, (int)(CLOCK_TICK_RATE / tone));
  usleep(1000 * duration);
  beeper_stop();
}

int get_corresponding_duration(string value_name) {
  return STD_DURATION / durations[value_name];
}

void play_f(const string note, const int octave, const string value) {
  int duration = get_corresponding_duration(value);
  play_beep(notes[note] * pow(2, octave), duration);
  usleep(1000 * duration);
}

int main(int argc, char **argv) {
  init();
  if (argc < 2)
    error("Usage speaker <file_name>");
  ifstream input(argv[1]);
  string note, value;
  int octave;
  while (!input.eof()) {
    input >> note;
    if (note == "P") {
      input >> value;
      usleep(1000 * get_corresponding_duration(value));
    } else {
      input >> octave >> value;
      play_f(note, octave, value);
    }
  }
  close(dev_console);
}