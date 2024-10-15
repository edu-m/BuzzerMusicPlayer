from mido import MidiFile
name = input()
midi = MidiFile(name)
ticks_per_beat = midi.ticks_per_beat

# Build a map of MIDI note numbers to note names and octaves
note_names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
midi_note_to_name = {}
for midi_note in range(128):
    octave = (midi_note // 12) - 1
    note = note_names[midi_note % 12]
    midi_note_to_name[midi_note] = (note, octave)

# Define duration mappings
duration_values = {
    4.0: 'w',   # whole note
    2.0: 'h',   # half note
    1.0: 'q',   # quarter note
    0.5: 'e',   # eighth note
    0.25: 's',  # sixteenth note
    0.125: 't', # thirty-second note
    0.0625: 'sf' # sixty-fourth note
}

for track in midi.tracks:
    abs_time = 0
    note_events = {}
    for msg in track:
        abs_time += msg.time
        if msg.type == 'note_on' and msg.velocity > 0:
            note_events[msg.note] = abs_time
        elif msg.type == 'note_off' or (msg.type == 'note_on' and msg.velocity == 0):
            if msg.note in note_events:
                start_time = note_events.pop(msg.note)
                duration_ticks = abs_time - start_time
                duration_beats = duration_ticks / ticks_per_beat

                # Find the closest duration value
                closest_duration = min(duration_values.keys(), key=lambda x: abs(x - duration_beats))
                duration_value = duration_values[closest_duration]

                note_name, octave = midi_note_to_name[msg.note]
                print(f"{note_name} {octave} {duration_value}")
