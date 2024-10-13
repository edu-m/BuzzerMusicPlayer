# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++11 -O3 -Wextra

# Include directories
INCLUDE_DIRS = src/include src/include/NotePlayer src/include/SoundPlayer src/include/Speaker
INCLUDES = $(foreach dir, $(INCLUDE_DIRS), -I$(dir))
CXXFLAGS += $(INCLUDES)
BUILD_DIR = build

# Libraries
LIBS = -lportaudio -lm

# VPATH for source files
VPATH = src:src/include/NotePlayer:src/include/SoundPlayer:src/include/Speaker

# Source files for 'speaker' executable
SPEAKER_SOURCES = main.cpp \
                  speaker.cpp \
				  noteplayer.cpp 
# Source files for 'speaker_soundcard' executable
SPEAKER_SOUNDCARD_SOURCES = main_soundcard.cpp \
                            noteplayer.cpp \
                            soundplayer.cpp \
							noteplayer_alsa.cpp \
							speaker.cpp

# Object directory
OBJDIR = src/obj

# Object files for 'speaker'
SPEAKER_OBJECTS = $(addprefix $(OBJDIR)/, $(SPEAKER_SOURCES:.cpp=.o))

# Object files for 'speaker_soundcard'
SPEAKER_SOUNDCARD_OBJECTS = $(addprefix $(OBJDIR)/, $(SPEAKER_SOUNDCARD_SOURCES:.cpp=.o))

# Dependency files
DEPFLAGS = -MMD -MP
DEPS = $(wildcard $(OBJDIR)/*.d)

# Target executables
TARGETS = $(BUILD_DIR)/speaker $(BUILD_DIR)/speaker_soundcard

# Default target
all: $(TARGETS)

# Build 'speaker' executable
$(BUILD_DIR)/speaker: $(SPEAKER_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(SPEAKER_OBJECTS)

# Build 'speaker_soundcard' executable
$(BUILD_DIR)/speaker_soundcard: $(SPEAKER_SOUNDCARD_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(SPEAKER_SOUNDCARD_OBJECTS) $(LIBS)

# Pattern rule to compile .cpp files to .o files in obj directory
 $(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Include dependency files
-include $(DEPS)

# Clean up
clean:
	rm -f $(BUILD_DIR)/speaker $(BUILD_DIR)/speaker_soundcard
	rm -rf $(OBJDIR)