# ─────────────────────────────────────────────────────────────────────────────
# Compiler and Flags
# ─────────────────────────────────────────────────────────────────────────────
CXX       = g++
CXXFLAGS  = -Wall -std=c++20 -O3 -Wextra
DEPFLAGS  = -MMD -MP  # Generate dependency info

# Libraries to link
LDLIBS    = -lportaudio -lm -lncurses

# ─────────────────────────────────────────────────────────────────────────────
# Include Directories
# ─────────────────────────────────────────────────────────────────────────────
# Adjust or add/remove directories as needed:
INCLUDE_DIRS = src/include \
               src/include/NotePlayer \
               src/include/SoundPlayer \
               src/include/Speaker \
               src/include/NcursesDrawer

CXXFLAGS += $(foreach dir, $(INCLUDE_DIRS), -I$(dir))

# ─────────────────────────────────────────────────────────────────────────────
# Paths and Directories
# ─────────────────────────────────────────────────────────────────────────────
VPATH     = src:src/include/NotePlayer:src/include/SoundPlayer:src/include/Speaker:src/include/NcursesDrawer
OBJDIR    = src/obj
BUILD_DIR = build

# ─────────────────────────────────────────────────────────────────────────────
# Source Files
# ─────────────────────────────────────────────────────────────────────────────
# 1) For the 'speaker' executable (no ncurses drawing):
SPEAKER_SOURCES = main.cpp \
                  speaker.cpp \
                  noteplayer.cpp \
                  NcursesDrawer.cpp

# 2) For the 'speaker_soundcard' executable (with ncurses drawing):
SPEAKER_SOUNDCARD_SOURCES = main_soundcard.cpp \
                            noteplayer.cpp \
                            NcursesDrawer.cpp \
                            soundplayer.cpp \
                            noteplayer_alsa.cpp \
                            speaker.cpp

# Derive object lists from source lists
SPEAKER_OBJECTS         = $(addprefix $(OBJDIR)/, $(SPEAKER_SOURCES:.cpp=.o))
SPEAKER_SOUNDCARD_OBJECTS = $(addprefix $(OBJDIR)/, $(SPEAKER_SOUNDCARD_SOURCES:.cpp=.o))

# Collect all .d files to include automatically
DEPS = $(wildcard $(OBJDIR)/*.d)

# Final targets
TARGETS = $(BUILD_DIR)/speaker \
          $(BUILD_DIR)/speaker_soundcard

# ─────────────────────────────────────────────────────────────────────────────
# Default Rule
# ─────────────────────────────────────────────────────────────────────────────
.PHONY: all clean
all: $(TARGETS)

# ─────────────────────────────────────────────────────────────────────────────
# Link Rules
# ─────────────────────────────────────────────────────────────────────────────
$(BUILD_DIR)/speaker: $(SPEAKER_OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/speaker_soundcard: $(SPEAKER_SOUNDCARD_OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

# ─────────────────────────────────────────────────────────────────────────────
# Compile Rules
# ─────────────────────────────────────────────────────────────────────────────
# Pattern rule to compile .cpp into .o
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# ─────────────────────────────────────────────────────────────────────────────
# Include generated dependency files
# ─────────────────────────────────────────────────────────────────────────────
-include $(DEPS)

# ─────────────────────────────────────────────────────────────────────────────
# Clean Rule
# ─────────────────────────────────────────────────────────────────────────────
clean:
	rm -f $(TARGETS)
	rm -rf $(OBJDIR)
