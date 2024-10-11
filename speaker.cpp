#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/input-event-codes.h>
#include <linux/kd.h>
#include <map>
#include <cmath>
#include <csignal>
#include <sys/time.h>
#include <unistd.h>
#include <cstdint>

class Speaker {
public:
    Speaker();
    ~Speaker();

    void sendTone(int tone);
    void stop();

private:
    int fd_;
    struct timeval tv_;
    struct EventVal {
        struct timeval t_val;
        uint16_t type;
        uint16_t code;
        uint32_t value;
    } ev_;
};

Speaker::Speaker() : fd_(-1) {
    fd_ = open("/dev/input/by-path/platform-pcspkr-event-spkr", O_WRONLY);
    if (fd_ == -1) {
        throw std::runtime_error("Failed to open speaker device");
    }

    gettimeofday(&tv_, nullptr);
    ev_.t_val = tv_;
    ev_.type = EV_SND;
    ev_.code = SND_TONE;
    ev_.value = 0;
}

Speaker::~Speaker() {
    if (fd_ != -1) {
        stop();
        close(fd_);
        fd_ = -1;
    }
}

void Speaker::sendTone(int tone) {
    ev_.value = tone;
    ssize_t result = write(fd_, &ev_, sizeof(ev_));
    if (result == -1) {
        throw std::runtime_error("Failed to send tone");
    }
}

void Speaker::stop() {
    sendTone(0); // Mute the speaker
}

class NotePlayer {
public:
    NotePlayer();
    int getDuration(const std::string& valueName) const;
    void play(const std::string& note, int octave, const std::string& value, Speaker& speaker);

private:
    static constexpr int STD_DURATION = 1000;
    std::map<std::string, int> durations_;
    std::map<std::string, float> notes_;
};

NotePlayer::NotePlayer() {
    durations_ = {
        {"w", 1}, {"h", 2}, {"q", 4}, {"e", 8},
        {"s", 16}, {"t", 32}, {"sf", 64}
    };

    notes_ = {
        {"C", 16.35}, {"C#", 17.32}, {"Db", 17.32},
        {"D", 18.35}, {"D#", 19.45}, {"Eb", 19.45},
        {"E", 20.6},  {"F", 21.83},  {"F#", 23.12},
        {"Gb", 23.12},{"G", 24.5},   {"G#", 25.96},
        {"Ab", 25.96},{"A", 27.5},   {"A#", 29.14},
        {"Bb", 29.14},{"B", 30.87}
    };
}

int NotePlayer::getDuration(const std::string& valueName) const {
    auto it = durations_.find(valueName);
    if (it != durations_.end()) {
        return STD_DURATION / it->second;
    } else {
        throw std::invalid_argument("Invalid duration value: " + valueName);
    }
}

void NotePlayer::play(const std::string& note, int octave, const std::string& value, Speaker& speaker) {
    int duration = getDuration(value);
    auto it = notes_.find(note);
    if (it != notes_.end()) {
        float frequency = it->second * std::pow(2, octave);
        speaker.sendTone(static_cast<int>(frequency));
        usleep(1000 * duration);
        speaker.stop();
        usleep(1000 * duration);
    } else {
        throw std::invalid_argument("Invalid note: " + note);
    }
}

Speaker* g_speaker = nullptr;

void handle_signal(int signum) {
    if (g_speaker != nullptr && signum == SIGINT) {
        g_speaker->stop();
    }
    exit(signum);
}

int main(int argc, char **argv) {
    try {
        std::signal(SIGINT, handle_signal);
        Speaker speaker;
        g_speaker = &speaker;
        NotePlayer notePlayer;

        if (argc < 2) {
            std::cerr << "Usage: speaker <file_name>" << std::endl;
            return EXIT_FAILURE;
        }

        std::ifstream input(argv[1]);
        if (!input) {
            std::cerr << "Failed to open input file: " << argv[1] << std::endl;
            return EXIT_FAILURE;
        }

        std::string note, value;
        int octave;
        while (input >> note) {
            if (note == "P") {
                input >> value;
                int duration = notePlayer.getDuration(value);
                usleep(1000 * duration);
            } else {
                input >> octave >> value;
                notePlayer.play(note, octave, value, speaker);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}

