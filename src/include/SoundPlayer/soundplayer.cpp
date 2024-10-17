#include "soundplayer.h"
#include <cmath>
#include <stdexcept>

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
    //*out++ = std::sinf(2 * M_PI * data->phase); // sine 
    // *out++ = 2 * (data->phase - (int)(data->phase + 0.5)); // sawtooth
    *out++ = 4 * (int)(data->phase)-2 * (int)(2 * data->phase) + 1; // square
    data->phase += data->frequency / SAMPLE_RATE;
    if (data->phase >= 1.0)
      data->phase -= 1.0;
  }

  return paContinue;
}
