#ifndef COOL_CONFIG_H
#define COOL_CONFIG_H

#define SAMPLING 32768  // 16-32k (obviously 2^n) for best tradeoffs.
#define SAMPLE_RATE 44100
#define STREAM_BUF 64
#define AUDIO_BUF SAMPLING
#define FFT_PILLARS 500
#define FPS 120

// Number of samples shown in the waveform
#define SHOWN_SAMPLES 6000

// FIRST_FREQ should be >0
#define FIRST_FREQ 20
#define LAST_FREQ 20000

// If peak decay should be enabled
#define DECAY_SMOOTHING 1
// Should decay be an interpolation between previous and current, or XX% of the previous?
#define DECAY_INTERPOLATE 0
#define DECAY_MULTIPLIER 0.9

// Multiplier for FFT height
#define SCALER 0.1

// Display what is believed to the the peak frequency and log RMS of the audio buffer
#define SHOW_STATS 0

typedef unsigned int uint;  // Because lazy
#endif
