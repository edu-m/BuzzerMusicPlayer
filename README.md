# BuzzerMusicPlayer
Plays music through the PC internal speaker

This program allows for very crude and simple sequencing/tracking through a file, each row containing three parameters:
- Note name
- Octave
- Note value

# File Structure
More specifically, note values are represented in American Notation, with only the initial of each value used (except for the sixty-fourth note represented as 'sf')
For example, a file could be similar to this:

C 4 w

P w

D 4 q

E 4 q
.
.
.
and so on

Note: when specifying a pause with P, do not put any octave number

# Usage
To compile, simply type 'make' in a terminal. You can also run 'make clean' to remove all executables.

There will be three executables:
- speaker: the main program, uses the pc speaker to produce sound
- speaker_old: should technically work the same as the above mentioned but the code is older and significantly less refined
- speaker_soundcard: instead of using the pc speaker, uses ALSA to emulate the sound

Running the program just requires one parameter, the input file:

./speaker input.txt
