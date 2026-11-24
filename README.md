# Alice
Alice is a simple single-header library providing OS abstraction and various utility

Great for projects such as emulators, demos, or learning.

## Headers:

#### alice.h
- Basic window managment
- Drawing surfaces
- Keyboard and mouse input
- A software renderer with basic graphics API such as rect or line drawing
- Colour manipulation functions
- Timing functions
- Portable, provides an easily extendable backending system to port it to new platforms

#### alice_audio.h
- Provides a software PCM audio buffer
- Includes a basic waveform synthesizer

#### alice_text.h
- Bitmap text renderer
- ASCII, UTF-8, and Shift-JIS
- Formatting and converters between formats
- Includes build-in 'Misaki Gothic' font (this can be disabled)

## Limitations
- Limited macOS support, it currently wraps SDL2 (which defeats the purpose of this library, but provides portability if need be)
- No emscriptem or WASM support (yet!)
- No hardware rendering support

## License
This library is under MIT License, you are free to do whatever you want with this including using it for non-commercial, and commercial projects!