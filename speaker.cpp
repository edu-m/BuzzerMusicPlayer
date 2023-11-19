#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/input-event-codes.h>
#include <linux/kd.h>
#include <map>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#define STD_DURATION 1000
#define SPKR_MUTE 0
#ifndef CLOCK_TICK_RATE
#define CLOCK_TICK_RATE 1193180
#endif
using namespace std;

typedef struct eventval {
  struct timeval t_val;
  u_int16_t type;
  u_int16_t code;
  u_int32_t value;

} event;

int pc_spkr_evnt_file = -1;
struct timeval tv;
struct eventval ev = {tv, EV_SND, SND_TONE, 0};
int sec = gettimeofday(&tv, NULL);
void *buf = &ev;

map<string, int> durations;
map<string, float> notes;

void send_tone(const int tone) {
  ev.value = tone;
  write(pc_spkr_evnt_file, buf, sizeof(ev));
}

void beeper_stop() { send_tone(SPKR_MUTE); }

void handle_signal(int signum) {
  if (pc_spkr_evnt_file >= 0 && signum == SIGINT) {
    beeper_stop();
    close(pc_spkr_evnt_file);
  }
  exit(signum);
}

void error(string what) {
  cout << what << endl;
  exit(EXIT_FAILURE);
}

void init() {
  signal(SIGINT, handle_signal);
  if ((pc_spkr_evnt_file = open("/dev/input/by-path/platform-pcspkr-event-spkr",
                                O_WRONLY)) == -1)
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
  send_tone(tone);
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
  close(pc_spkr_evnt_file);
}