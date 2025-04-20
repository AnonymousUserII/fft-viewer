#ifndef COOL_CONFIG_H
#define COOL_CONFIG_H

#define SHOW_WAVEFORM 1
#define SHOW_FFT 1

// Window config
#define LENGTH 1500
#define HEIGHT 1000
#define FFT_PILLARS (1500)
#define FPS 120
/// SDL Colors
#define BG ((SDL_Color) {0, 0, 0})
#define WAVE ((SDL_Color) {192, 192, 192})
#define BEAM ((SDL_Color) {128, 64, 64})

#define WAVEFORM_HEIGHT 300

// Lengths of buffers used for FFT analysis
// 16-32k (obviously 2^n) for best tradeoffs of time vs accuracy
// AUDIO_BUF <= SAMPLING
#define SAMPLING 32768
#define AUDIO_BUF 32768

// Number of samples shown in the waveform
#define SHOWN_SAMPLES (AUDIO_BUF)

#define UNMIRROR_FFT 0

// How many samples to get from song at a time, lower means smoother animations
#define STREAM_BUF 128

// Sample rate for microphone
#define SAMPLE_RATE 44100

// FIRST_FREQ > 0
// LAST_FREQ < SAMPLING
#define FIRST_FREQ 20
#define LAST_FREQ 20000

// If peak decay should be enabled
#define DECAY_SMOOTHING 1
// Should decay be an interpolation between previous and current, or constant fall from the previous?
#define DECAY_INTERPOLATE 0
#define DECAY_MULTIPLIER 0.9

// Multiplier for FFT height
#define SCALER 0.1

// Print out what is believed to the the peak frequency and log RMS of the audio buffer
#define SHOW_STATS 0

typedef unsigned int uint;  // Because lazy
#endif
