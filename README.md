# FFT Viewer
A limited waveform and FFT viewer using the SDL2 library.

It is currently hard-coded to take in 16-bit mono WAV files.

## Building
It requires the SDL2 library. Please figure that on your own.

## Using
This application will either take the input from your computer's default microphone
or take in a WAV file (with the aforementioned restrictions).

To use the microphone, pass in no arguments, i.e.
```sh
./fft
```

To use in file player mode, give in the path to the file,
```sh
./fft [file_path.wav]
```
When in file player mode, you can drop in other WAVs to play them instead.

### Note
For some reason, the playback sample rate does not change when dragging in files.
For the first track, sample rate will work, but will not be changed between files.
In effect, changing from 44.1kHz to 48kHz will result in a ~8% slowdown.

