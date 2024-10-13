#include "speaker.h"
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/kd.h>
#include <stdexcept>
#include <sys/time.h>
#include <unistd.h>

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