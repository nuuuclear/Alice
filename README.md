# Alice
Alice is a simple single-header library for all you graphical, input, and windowing needs.

Best for simple projects such as emulators, demos, or learning.

## What it provides

- A Window
- Cross-platform keyboard and mouse input
- A software renderer with basic graphics API such as rect or line drawing
- Portablility, provides an easily extendable backending system to port it to new platforms
- Colour manipulation functions

With optional:
- Audio header with audio API and a waveform synth
- Text header with a text renderer and format utilities

## Limitations
- Limited macOS support, it currently wraps SDL2 (which defeats the purpose of this library, but provides portability if need be)
- No hardware rendering support

## License
This library is under MIT License, you are free to do whatever you want with this including using it for non-commercial, and commercial projects!