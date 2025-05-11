#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <SDL.h>

#include "util.h"
#include "stuff.h"
#include "dfts.h"

char holdAudioStream = 0;
char doMusic;
char stereo = 0;

short audioStreamIn[STREAM_BUF * 2] = {0};
short audioBufferOriginalIn[AUDIO_BUF] = {0};
short audioBufferIn[AUDIO_BUF] = {0};
short audioBufferInR[AUDIO_BUF] = {0};
double complex originalIn[SAMPLING] = {0};
double complex originalInR[SAMPLING] = {0};
double complex fftIn[SAMPLING] = {0};
double complex fftInR[SAMPLING] = {0};
double complex fftOut[SAMPLING] = {0};
double complex fftOutR[SAMPLING] = {0};
double complex lastFftOut[SAMPLING] = {0};
double complex lastFftOutR[SAMPLING] = {0};

struct AudioData {
    Uint32 length;
    Uint8 *pos;
};

// cos^2(pi * x / L)
double reduction(double streamDatum, Uint32 streamDatumIndex, Uint32 streamLength) {
    if (streamDatumIndex == streamLength - 1 || streamDatumIndex == 0) return 0;

    Uint32 half = streamLength / 2;
    double cos_coeff = M_PI * (streamDatumIndex - half) / streamLength;
    double value = streamDatum * cos(cos_coeff) * cos(cos_coeff);
    return value;
}

double eq(double sample, Uint32 frequency) {
    return sample;
    return sample * (1 + 0.1*(log(frequency) - log(500)));
}

void AudioCallback(void *userData, Uint8 *stream, Uint32 streamLength) {
    struct AudioData *audio = (struct AudioData *) userData;

    Uint32 length = streamLength;
    if (doMusic) {
        if (!audio->length) return;
        if (!holdAudioStream) {
            length = min(streamLength, audio->length);
            SDL_memcpy(stream, audio->pos, length);
            audio->pos += length;
            audio->length -= length;
        } else {
            SDL_memcpy(stream, audioBufferOriginalIn, length);
        }
    }

    SDL_memcpy(audioStreamIn, stream, length);
    length /= 2;  // No idea why this is needed
    if (!stereo) {
        Uint32 newStartIndex = AUDIO_BUF - length;
        for (Uint32 i = 0; i < newStartIndex; i++) {
            // Shift audioBufferIn down
            audioBufferIn[i] = audioBufferIn[i + length];
        }
        SDL_memcpy(audioBufferIn + newStartIndex, audioStreamIn, length * 2);
        SDL_memcpy(audioBufferOriginalIn, audioBufferIn, AUDIO_BUF);
    } else {
        for (Uint32 i = 0; i < AUDIO_BUF - length; i++) {
            audioBufferOriginalIn[i] = audioBufferOriginalIn[i + length];
        }
        SDL_memcpy(audioBufferOriginalIn + AUDIO_BUF - length, audioStreamIn, length * 2);

        // Shift audioBuffers down
        Uint32 newStartIndex = AUDIO_BUF - length / 2;
        for (Uint32 i = 0; i < newStartIndex; i++) {
            audioBufferIn[i] = audioBufferIn[i + length / 2];
            audioBufferInR[i] = audioBufferInR[i + length / 2];
        }
        // Add new audioStream data
        for (Uint32 i = 0; i < length; i += 2) {
            audioBufferIn[newStartIndex + i / 2] = audioStreamIn[i];
            audioBufferInR[newStartIndex + i / 2] = audioStreamIn[i + 1];
        }
    }
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "You done goof\'d\n");
        return -1;
    }

    if (argc > 2 || (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))) {
        fprintf(stderr, "Usage: %s [audio file]\n", argv[0]);
        fprintf(stderr, "File must be a 16-bit WAV\n");
        return 1;
    }

    doMusic = argc != 1;

    SDL_AudioSpec wavSpec;
    struct AudioData audio;
    Uint32 wavLength;
    Uint8 *wavBuffer;

    char *playingFile = argc == 2 ? argv[1] : "(Your default microphone)";
    SDL_AudioDeviceID deviceId;
    printf("file: %s\n", playingFile);

    if (doMusic) {
        /*
         * File must be of WAV file type, mono channel, 16-bit depth, [only tested for 44.1 kHz sample rate]
         * i.e. CD-quality mono WAV only
         */
        if (SDL_LoadWAV(playingFile, &wavSpec, &wavBuffer, &wavLength) == NULL) {
            fprintf(stderr, "File no load..\n");
            return 1;
        }
        printf("Audio File Info:\n"
            "   Channels: %d\n"
            "Sample Rate: %d\n"
            "    Samples: %d\n", wavSpec.channels, wavSpec.freq, wavLength);

        if (wavSpec.channels == 2)
            stereo = 1;
        audio.pos = wavBuffer;
        audio.length = wavLength;

        wavSpec.samples = STREAM_BUF;
        wavSpec.callback = (void *) AudioCallback;
        wavSpec.userdata = &audio;

        deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
        if (!deviceId) {
            printf("Device no find..\n");
            return 2;
        }
        // SDL_QueueAudio(deviceId, wavBuffer, wavLength);

    } else {
        SDL_AudioSpec gotten;
        wavSpec.freq = SAMPLE_RATE;
        wavSpec.samples = STREAM_BUF;
        wavSpec.format = AUDIO_S16SYS;
        wavSpec.callback = (void *) AudioCallback;
        deviceId = SDL_OpenAudioDevice(NULL, 1, &wavSpec, &gotten, 0);
        SDL_PauseAudioDevice(deviceId, 0);
    }

    SDL_Window *window = SDL_CreateWindow(
        "FFT Viewer", SDL_WINDOWPOS_CENTERED_DISPLAY(0), SDL_WINDOWPOS_CENTERED_DISPLAY(0),
        LENGTH, HEIGHT, SDL_WINDOW_SHOWN
    );
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    char autoPaused = 0;
    char isPaused = 0;
    char isRunning = 1;
    while (isRunning) {
        const Uint32 frameStart = SDL_GetTicks();
        SDL_SetRenderDrawColor(renderer, BG.r, BG.g, BG.b, BG.a);
        SDL_RenderClear(renderer);

        if (doMusic) {
            if (audio.length == 0 || wavLength < audio.length) {
                isPaused = 1;
                autoPaused = 1;
                audio.pos = wavBuffer + wavLength;
                audio.length = 0;
            }
            SDL_Rect progressBar = {0, HEIGHT - 5, ((double) (wavLength - audio.length) / wavLength) * LENGTH, 5};
            SDL_SetRenderDrawColor(renderer, WAVE.r, WAVE.g, WAVE.b, WAVE.a);
            SDL_RenderFillRect(renderer, &progressBar);
        }

        // Reduce sample for nice windowing (apply windowing function)
        for (uint sample = 0; sample < SAMPLING; sample++) {
            fftIn[sample] = (double complex) (double) audioBufferIn[sample % AUDIO_BUF] / -INT16_MIN / 2;

            // Keep the pre-windowed waveform for displaying
            originalIn[sample] = fftIn[sample];

            // Reduce each looped buffer
            fftIn[sample] = reduction(fftIn[sample], sample % AUDIO_BUF, AUDIO_BUF);

            // Perform reduction on whole sample
            //fftIn[sample] = reduction(fftIn[sample], sample, SAMPLING);
            if (stereo) {
                fftInR[sample] = (double complex) (double) audioBufferInR[sample % AUDIO_BUF] / -INT16_MIN / 2;
                originalInR[sample] = fftInR[sample];
                fftInR[sample] = reduction(fftInR[sample], sample % AUDIO_BUF, AUDIO_BUF);
            }
        }

#if SHOW_WAVEFORM
        SDL_Color waveColor = stereo ? WAVE_L : WAVE;
        float waveHeight = WAVEFORM_HEIGHT;
        if (stereo)
            waveHeight /= 2.;
        const double horizontalSeparator = (double) LENGTH / SHOWN_SAMPLES;
        float yOffset = stereo ? - waveHeight / 2 - 1 : 0;
        float prevX = 0.;
        float prevY = HEIGHT / 2 + yOffset;
        SDL_SetRenderDrawColor(renderer, waveColor.r, waveColor.g, waveColor.b, waveColor.a);
        for (Uint32 frame = 1; frame < SHOWN_SAMPLES; frame++) {
            float thisX = (float) frame * horizontalSeparator;
            float thisY = (float) originalIn[SAMPLING - SHOWN_SAMPLES + frame] * waveHeight + HEIGHT / 2;
            thisY += yOffset;
            SDL_RenderDrawLineF(renderer, prevX, prevY, thisX, thisY);
            prevX = thisX;
            prevY = thisY;
        }

        if (stereo) {
            yOffset = stereo ? waveHeight / 2 : 0;
            prevX = 0.;
            prevY = HEIGHT / 2 + yOffset;
            SDL_SetRenderDrawColor(renderer, WAVE_R.r, WAVE_R.g, WAVE_R.b, WAVE_R.a);
            for (Uint32 frame = 1; frame < SHOWN_SAMPLES; frame++) {
                float thisX = (float) frame * horizontalSeparator;
                float thisY = (float) originalInR[SAMPLING - SHOWN_SAMPLES + frame] * waveHeight + HEIGHT / 2;
                thisY += yOffset;
                SDL_RenderDrawLineF(renderer, prevX, prevY, thisX, thisY);
                prevX = thisX;
                prevY = thisY;
            }
        }
#endif

#if SHOW_FFT
        SDL_SetRenderDrawColor(renderer, BEAM.r, BEAM.g, BEAM.b, BEAM.a);

        fft(fftIn, fftOut, SAMPLING);
        if (stereo)
            fft(fftInR, fftOutR, SAMPLING);

        const Uint16 base = FIRST_FREQ;
        const Uint16 last = LAST_FREQ;
        const double xInc = (double) LENGTH / FFT_PILLARS;

        // Attempt a smooth peak decay
        if (DECAY_SMOOTHING) {
            for (int i = 0; i < SAMPLING; i++) {
                if (temperDouble(cabs(fftOut[i])) < temperDouble(cabs(lastFftOut[i]))) {
                    if (DECAY_INTERPOLATE)
                        fftOut[i] = (lastFftOut[i] + fftOut[i]) / 2;
                    else
                        fftOut[i] = lastFftOut[i] * DECAY_MULTIPLIER;
                }
                if (stereo && temperDouble(cabs(fftOutR[i])) < temperDouble(cabs(lastFftOutR[i]))) {
                    if (DECAY_INTERPOLATE)
                        fftOutR[i] = (lastFftOutR[i] + fftOutR[i]) / 2;
                    else
                        fftOutR[i] = lastFftOutR[i] * DECAY_MULTIPLIER;
                }
            }
        }

        const double nextStep = (double) (last - base) / FFT_PILLARS;  // For linear
        const double oneStep = pow((double) last / base, (double) 1 / FFT_PILLARS); // For logarithmic
        double workingFreq = base;
        double nextFreq = base;
        for (double x = 0; x < LENGTH; x += xInc, workingFreq = nextFreq) {
            if (LOG_PLOT)
                nextFreq = workingFreq * oneStep;
            else
                nextFreq = workingFreq + nextStep;

            if (workingFreq > SAMPLING / 2)
                continue;

            double maxSample = temperDouble(cabs(fftOut[(Uint16) workingFreq + 1]));
            double maxSampleR = temperDouble(cabs(fftOutR[(Uint16) workingFreq + 1]));
            Uint16 nextFreq = workingFreq + nextStep;  // Linear
            if (LOG_PLOT) {
                nextFreq = workingFreq * oneStep;  // Logarithmic
            }
            for (Uint16 f = workingFreq + 1; f < nextFreq; f++) {
                complex double value = fftOut[f];
                complex double valueR = fftOutR[f];
                maxSample = maxd(maxSample, temperDouble(cabs(value)));
                maxSampleR = maxd(maxSampleR, temperDouble(cabs(valueR)));
            }

            if (stereo && maxSampleR > 1) {
                maxSampleR = eq(log(maxSampleR), workingFreq) * SCALER;

                SDL_Rect rect;
                rect.x = x;
                rect.y = ceil(HEIGHT - maxSampleR * HEIGHT / 2) - 5;
                rect.w = xInc;
                rect.h = maxSampleR * HEIGHT / 2;
                SDL_SetRenderDrawColor(renderer, BEAM_R.r, BEAM_R.g, BEAM_R.b, BEAM_R.a);
                SDL_RenderFillRect(renderer, &rect);
            }
            if (maxSample > 1) {
                // Apply blanket eq to reduce bass height, then scale height
                maxSample = eq(log(maxSample), workingFreq) * SCALER;

                SDL_Rect rect;
                rect.x = x;
                rect.y = ceil(HEIGHT - maxSample * HEIGHT / 2) - 5;
                rect.w = xInc;
                rect.h = maxSample * HEIGHT / 2;
                
                SDL_SetRenderDrawColor(renderer, BEAM.r, BEAM.g, BEAM.b, BEAM.a);
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        // Copy the current FFT to previous FFT
        for (int i = 0; i < SAMPLING; i++) {
            lastFftOut[i] = fftOut[i];
            lastFftOutR[i] = fftOutR[i];
        }

    #if SHOW_STATS
        if (!isPaused) {
            Uint16 maxFreq = base;
            double totalSumSqrd = 0;
            for (Uint16 f = base; f < min(last, SAMPLING / 2); f++) {
                double mag = temperDouble(cabs(fftOut[f]));
                totalSumSqrd += mag * mag;
                maxFreq = (mag > temperDouble(cabs(fftOut[maxFreq]))) ? f : maxFreq;
            }
            double totalMeanSqrd = totalSumSqrd / (last - base);
            double blockLinearRms = sqrt(totalMeanSqrd);
            double blockLogRms = 20 * log10(blockLinearRms);  // dBFS of sample

            printf(
                "Max: %5dHz (%7.5lf)\tRMS: %7.3lf\n",
                maxFreq * wavSpec.freq / SAMPLING, temperDouble(cabs(fftOut[maxFreq])), blockLogRms
            );
        }
    #endif
#endif // SHOW_FFT

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    isRunning = 0;
                    break;

                case SDL_DROPFILE:
                    if (!doMusic) break;

                    SDL_FreeWAV(wavBuffer);
                    char *oldFile = playingFile;
                    playingFile = event.drop.file;
                    if (SDL_LoadWAV(playingFile, &wavSpec, &wavBuffer, &wavLength) == NULL) {
                        printf("File no load... (probably invalid)\n");

                        // Revert to original file
                        playingFile = oldFile;
                        SDL_LoadWAV(playingFile, &wavSpec, &wavBuffer, &wavLength);
                    }
                    printf("Audio File Info:\n"
                           "   Channels: %d\n"
                           "Sample Rate: %d\n"
                           "    Samples: %d\n", wavSpec.channels, wavSpec.freq, wavLength);

                    audio.pos = wavBuffer;
                    audio.length = wavLength;
                    stereo = wavSpec.channels == 2;

                    wavSpec.samples = STREAM_BUF;
                    wavSpec.callback = (void *) AudioCallback;
                    wavSpec.userdata = &audio;

                    // Reopen device to reset sample rate
                    SDL_CloseAudioDevice(deviceId);
                    deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

                    isPaused = 0;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_SPACE:
                            isPaused = !isPaused;
                            break;
                        case SDLK_h:
                            if (!doMusic) break;
                            holdAudioStream = !holdAudioStream;
                            break;
                        case SDLK_LEFT:
                            if (!doMusic) break;
                            audio.pos -= wavSpec.freq * 2 * 5;
                            audio.length += wavSpec.freq * 2 * 5;
                            if (audio.length > wavLength) {
                                const uint overshoot = audio.length - wavLength;
                                audio.length = wavLength;
                                audio.pos += overshoot;
                            }
                            if (autoPaused) {
                                autoPaused = 0;
                                isPaused = 0;
                            }
                            break;
                        case SDLK_RIGHT:
                            if (!doMusic) break;
                            audio.pos += wavSpec.freq * 2 * 5;
                            audio.length -= wavSpec.freq * 2 * 5;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
        SDL_PauseAudioDevice(deviceId, isPaused);
        SDL_RenderPresent(renderer);
        const Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < 1000 / FPS) {
            SDL_Delay(1000 / FPS - frameTime);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_CloseAudioDevice(deviceId);
    if (doMusic) SDL_FreeWAV(wavBuffer);
    SDL_Quit();
    return 0;
}
