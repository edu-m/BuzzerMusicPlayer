CXX=g++
CXXFLAGS=-O3 -march=native -g -Wall -Wextra -pedantic
SRC_DIR=src
BUILD_DIR=build
BINS=$(BUILD_DIR)/speaker $(BUILD_DIR)/speaker_old $(BUILD_DIR)/speaker_soundcard

all: $(BINS)

$(BUILD_DIR)/speaker_soundcard: LDLIBS=-lportaudio

$(BUILD_DIR)/%: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDLIBS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean: 
	rm -f $(BINS)
