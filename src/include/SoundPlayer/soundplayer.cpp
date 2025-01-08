#include "soundplayer.h"
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>

std::function<double(double)> SoundPlayer::waveFunc = nullptr;

inline double sineWave(double phase) { return std::sin(2.0 * M_PI * phase); }

inline double sawtoothWave(double phase) {
  return 2.0 * (phase - static_cast<int>(phase + 0.5));
}

inline double squareWave(double phase) {
  return 4.0 * static_cast<int>(phase) - 2.0 * static_cast<int>(2 * phase) +
         1.0;
}

inline double triangleWave(double phase) {
  return 4.0 * std::abs(phase - std::floor(phase + 0.75) + 0.25) - 1.0;
}

SoundPlayer::SoundPlayer(char type) : stream_(nullptr), data_{0.0, 0.0} {
  PaError err = Pa_Initialize();
  if (err != paNoError) {
    throw std::runtime_error("PortAudio initialization failed");
  }
  switch (type) {
  case 'S':
    std::cout << "chosen sine" << std::endl;
    waveFunc = sineWave;
    break;
  case 'W':
    std::cout << "chosen sawtooth" << std::endl;
    waveFunc = sawtoothWave;
    break;
  case 'Q':
    std::cout << "chosen square" << std::endl;
    waveFunc = squareWave;
    break;
  case 'T':
    std::cout << "chosen triangle" << std::endl;
    waveFunc = triangleWave;
    break;
  default:
    std::cout << "chosen sine" << std::endl;
    waveFunc = sineWave;
    break;
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

  PaError err = Pa_OpenStream(&stream_, nullptr, &outputParameters, SAMPLE_RATE,
                              paFramesPerBufferUnspecified, paClipOff,
                              paCallback, &data_);
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

int SoundPlayer::paCallback(const void * /*inputBuffer*/, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo * /*timeInfo*/,
                            PaStreamCallbackFlags /*statusFlags*/,
                            void *userData) {
  float *out = static_cast<float *>(outputBuffer);
  PaData *data = static_cast<PaData *>(userData);

  for (unsigned int i = 0; i < framesPerBuffer; ++i) {
    *out++ = waveFunc(data->phase);
    data->phase += data->frequency / SAMPLE_RATE;
    if (data->phase >= 1.0)
      data->phase -= 1.0;
  }

  return paContinue;
}
