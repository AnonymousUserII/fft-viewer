# FFT Viewer
A limited waveform and FFT viewer using the SDL2 library.

It is currently hard-coded to take in 16-bit WAV files.

## Building
> It requires the SDL2 library...

If you don't have cmake installed, you'll have a lot of fun trying to make that work.

```sh
git clone https://github.com/libsdl-org/SDL.git --depth=1 --branch SDL2 vendored/SDL
cmake -S . -B build
cmake --build build
```
The executable will be at `build/fft`.

## Using
This application will either take the input from your computer's default microphone
or take in a 16-bit WAV file.

To use the microphone, pass in no arguments, i.e.
```sh
./fft
```

To use in file player mode, pass in the path to the file,
```sh
./fft [file_path.wav]
```
When in file player mode, you can drop other WAVs onto the window to switch to them.

