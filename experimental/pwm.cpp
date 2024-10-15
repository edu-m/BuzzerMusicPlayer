#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>

int console_fd = -1;
bool keep_running = true;

void cleanup() {
    if (console_fd != -1) {
        ioctl(console_fd, KIOCSOUND, 0);
        close(console_fd);
        console_fd = -1;
    }
}

void handle_signal(int signum) {
    keep_running = false;
    cleanup();
    exit(signum);
}

void set_speaker_state(bool on) {
    if (console_fd == -1) {
        console_fd = open("/dev/console", O_WRONLY);
        if (console_fd == -1) {
            std::cerr << "Failed to open /dev/console" << std::endl;
            return;
        }
    }

    int value = on ? 1193180 / 440 : 0; // 440 Hz as base frequency
    if (ioctl(console_fd, KIOCSOUND, value) < 0) {
        std::cerr << "Failed to set speaker state" << std::endl;
        cleanup();
    }
}

void play_pwm_sawtooth_sweep(int duration_ms, int start_freq, int end_freq) {
    auto start_time = std::chrono::steady_clock::now();
    int total_duration_us = duration_ms * 1000;

    // Parameters for PWM
    int pwm_frequency = 40000; // 40 kHz PWM frequency
    int pwm_period_us = 1000000 / pwm_frequency;

    while (keep_running) {
        auto elapsed_time = std::chrono::steady_clock::now() - start_time;
        int elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_time).count();
        if (elapsed_us >= total_duration_us) break;

        // Calculate the current frequency in the sweep
        double t = static_cast<double>(elapsed_us) / total_duration_us;
        int current_freq = start_freq + t * (end_freq - start_freq);

        // Calculate duty cycle for sawtooth waveform
        double duty_cycle = fmod(t * pwm_frequency / current_freq, 1.0);

        // On-time and off-time in microseconds
        int on_time_us = static_cast<int>(duty_cycle * pwm_period_us);
        int off_time_us = pwm_period_us - on_time_us;

        // Turn speaker on
        set_speaker_state(true);
        if (on_time_us > 0)
            std::this_thread::sleep_for(std::chrono::microseconds(on_time_us));

        // Turn speaker off
        set_speaker_state(false);
        if (off_time_us > 0)
            std::this_thread::sleep_for(std::chrono::microseconds(off_time_us));
    }

    // Ensure the speaker is turned off at the end
    set_speaker_state(false);
}

int main() {
    std::signal(SIGINT, handle_signal);

    int duration_ms = 5000;   // Duration of the sweep in milliseconds
    int start_freq = 100;     // Starting frequency in Hz
    int end_freq = 1000;      // Ending frequency in Hz

    std::cout << "Playing sawtooth sweep from " << start_freq << " Hz to " << end_freq << " Hz" << std::endl;

    play_pwm_sawtooth_sweep(duration_ms, start_freq, end_freq);

    cleanup();
    return 0;
}
