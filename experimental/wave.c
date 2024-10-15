#include "notes.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define SAMPLE_RATE 48000.0
#define OCTAVE 4
enum types { SQUARE, SINE, SAWTOOTH };

void error(const char *what) {
  fprintf(stderr, "%s\n", what);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  int wave_type = SINE;
  double phase = 0;
  double frequency = NOTE_B * pow(2, OCTAVE);
  if (argc < 2)
    fprintf(stderr, "no wave type specified. Defaulting to sine\n");
  else
    wave_type = atoi(argv[1]);

  for (int i = 0; i < 400; ++i) {
    switch (wave_type) {
    case SQUARE:
      printf("%d\n", 4 * (int)(phase)-2 * (int)(2 * phase) + 1);
      break;
    case SINE:
      printf("%f\n", sin(2 * M_PI * phase));
      break;
    case SAWTOOTH:
      printf("%f\n", 2 * (phase - (int)(phase + 0.5)));
      break;
    }
    phase += frequency / SAMPLE_RATE;
    if (phase >= 1.0)
      phase -= 1.0;
  }
}