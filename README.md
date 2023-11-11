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
Usage is very simple, just compile and run specifying the file to be read
