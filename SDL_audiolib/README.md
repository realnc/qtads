SDL_audiolib - An audio decoding, resampling and mixing library.

This is a small and simple to use C++ library for playing various audio 
formats. It is a thin (-ish) wrapper around existing resampling (like SRC or 
SoX) and decoding libraries (like libmpg123 or libvorbis.)

I wrote it as my personal replacement for SDL_mixer, due to SDL_mixer's lack 
of multiple music streams, limited audio format support and poor resampling 
quality.

The API and ABI are not finalized (not sure if they'll ever be) and it's 
C++-only at the moment. You will find what looks to be the beginnings of an 
SDL_mixer drop-in replacement implementation, but it's not actually 
implemented to any useful extent and will probably be removed in the future.

As the name implies, it uses [SDL](http://www.libsdl.org) to access the audio 
hardware.

Most popular audio formats are supported:

  * Vorbis
  * Opus
  * MP3
  * Musepack
  * WAV (and related formats through libsndfile)
  * FLAC (through libsndfile)
  * MIDI (FluidSynth, BASSMIDI, WildMIDI, libADLMIDI)
  * MOD-based music formats (libopenmpt, libxmp, libmodplug)

You can also write your own decoders and resamplers by subclassing 
`Aulib::Decoder` and `Aulib::Resampler`.

Using the library is fairly simple:

```c++
#include <Aulib/DecoderVorbis.h>
#include <Aulib/ResamplerSpeex.h>
#include <Aulib/Stream.h>
#include <SDL.h>
#include <iostream>

// The library uses std::chrono for durations, seeking and fading.
using namespace std::chrono_literals;

int main()
{
    // Initialize the SDL_audiolib library. Set the output sample rate to
    // 44.1kHz, the audio format to 16-bit signed, use 2 output channels
    // (stereo), and an 8kB output buffer.
    if (Aulib::init(44100, AUDIO_S16SYS, 2, 8192) != 0) {
        std::cerr << "Couldn't initialize audio: " << SDL_GetError() << '\n';
        return EXIT_FAILURE;
    }

    // Create an audio stream that will play our Vorbis file using a Vorbis
    // decoder and a Speex resampler.
    Aulib::Stream music("music.ogg",
                        std::make_unique<Aulib::DecoderVorbis>(),
                        std::make_unique<Aulib::ResamplerSpeex>());

    // Play it once with a fade-in of 700 milliseconds.
    music.play(1, 700ms);

    // Wait while the music is still playing.
    while (music.isPlaying()) {
        SDL_Delay(200);
    }
}
```

For further details, read the 
[API reference](http://realnc.github.io/SDL_audiolib).
