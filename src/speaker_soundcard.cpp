#include <iostream>
#include <fstream>
#include <map>
#include <cmath>
#include <csignal>
#include <portaudio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class SoundPlayer {
public:
    SoundPlayer();
    ~SoundPlayer();

    void playTone(double frequency, int duration_ms);

private:
    static int paCallback(const void* inputBuffer, void* outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags, void* userData);

    PaStream* stream_;

    struct PaData {
        double phase;
        double frequency;
    } data_;
};

SoundPlayer::SoundPlayer() : stream_(nullptr), data_{0.0, 0.0} {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        throw std::runtime_error("PortAudio initialization failed");
    }
}

SoundPlayer::~SoundPlayer() {
    if (stream_) {
        Pa_CloseStream(stream_);
    }
    Pa_Terminate();
}

void SoundPlayer::playTone(double frequency, int duration_ms) {
    data_.phase = 0.0;
    data_.frequency = frequency;

    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        throw std::runtime_error("No default output device");
    }
    outputParameters.channelCount = 1;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency =
        Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenStream(&stream_, nullptr, &outputParameters, 44100,
                                paFramesPerBufferUnspecified, paClipOff, paCallback, &data_);
    if (err != paNoError) {
        throw std::runtime_error("Failed to open stream");
    }

    err = Pa_StartStream(stream_);
    if (err != paNoError) {
        throw std::runtime_error("Failed to start stream");
    }

    Pa_Sleep(duration_ms);

    err = Pa_StopStream(stream_);
    if (err != paNoError) {
        throw std::runtime_error("Failed to stop stream");
    }

    err = Pa_CloseStream(stream_);
    if (err != paNoError) {
        throw std::runtime_error("Failed to close stream");
    }

    stream_ = nullptr; // Reset the stream pointer
}

int SoundPlayer::paCallback(const void* /*inputBuffer*/, void* outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* /*timeInfo*/,
                            PaStreamCallbackFlags /*statusFlags*/, void* userData) {
    float* out = static_cast<float*>(outputBuffer);
    PaData* data = static_cast<PaData*>(userData);

    for (unsigned int i = 0; i < framesPerBuffer; ++i) {
        *out++ = static_cast<float>(std::sin(2 * M_PI * data->phase));
        data->phase += data->frequency / 44100.0;
        if (data->phase >= 1.0) data->phase -= 1.0;
    }

    return paContinue;
}

class NotePlayer {
public:
    NotePlayer();
    int getDuration(const std::string& valueName) const;
    void play(const std::string& note, int octave, const std::string& value, SoundPlayer& player);

private:
    static constexpr int STD_DURATION = 1000;
    std::map<std::string, int> durations_;
    std::map<std::string, double> notes_;
};

NotePlayer::NotePlayer() {
    durations_ = {
        {"w", 1}, {"h", 2}, {"q", 4}, {"e", 8},
        {"s", 16}, {"t", 32}, {"sf", 64}
    };

    notes_ = {
        {"C", 16.35}, {"C#", 17.32}, {"Db", 17.32},
        {"D", 18.35}, {"D#", 19.45}, {"Eb", 19.45},
        {"E", 20.60}, {"F", 21.83}, {"F#", 23.12},
        {"Gb", 23.12},{"G", 24.50}, {"G#", 25.96},
        {"Ab", 25.96},{"A", 27.50}, {"A#", 29.14},
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

void NotePlayer::play(const std::string& note, int octave, const std::string& value, SoundPlayer& player) {
    int duration = getDuration(value);
    auto it = notes_.find(note);
    if (it != notes_.end()) {
        double frequency = it->second * std::pow(2, octave);
        player.playTone(frequency, duration);
        Pa_Sleep(duration); // Rest between notes
    } else {
        throw std::invalid_argument("Invalid note: " + note);
    }
}

void handle_signal(int signum) {
    Pa_Terminate();
    exit(signum);
}

int main(int argc, char **argv) {
    try {
        std::signal(SIGINT, handle_signal);
        SoundPlayer player;
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
                Pa_Sleep(duration);
            } else {
                input >> octave >> value;
                notePlayer.play(note, octave, value, player);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}

