# Warcraft 3 Reforged Chat Wheel

A Dota 2-style chat wheel overlay for Warcraft 3 Reforged, allowing quick communication through predefined messages.

## Features

- **Quick Access**: Press `Ctrl+G` to activate the chat wheel
- **8 Preset Messages**: Common phrases for team coordination
- **Visual Overlay**: Transparent wheel overlay that works in fullscreen
- **Mouse Selection**: Simply hover over your desired message
- **Auto-send**: Release `Ctrl` to automatically send the selected message to all chat

## Download

Download the latest release from the [Releases](https://github.com/patrickjaja/wc3-reforged-chat-wheel-c/releases) page.

## Usage

1. Download `chatwheel.exe` from the releases
2. Run the executable (no installation required)
3. Start Warcraft 3 Reforged
4. In-game, press and hold `Ctrl+G` to show the chat wheel
5. Move your mouse to highlight a message
6. Release `Ctrl` to send the message

## Available Messages

- Well played!
- Get back!
- Push now!
- Need help!
- On my way
- Enemy missing!
- Good game
- Thanks!

## Building from Source

### Prerequisites

- MinGW-w64 compiler (for cross-compilation on Linux)
- Make
- GDI+ development headers

### Linux Build Instructions

```bash
# Clone the repository
git clone https://github.com/patrickjaja/wc3-reforged-chat-wheel-c.git
cd wc3-reforged-chat-wheel-c

# Build the executable
make

# The executable will be in build/chatwheel.exe
```

### Windows Build Instructions

If building on Windows with MinGW:

```bash
# Adjust the Makefile CXX variable to use your local MinGW compiler
# Then run:
make
```

## Technical Details

- **Language**: C++17
- **UI Framework**: Windows GDI+ for rendering
- **Input**: Windows API for hotkey registration and input simulation
- **Size**: ~220KB standalone executable (statically linked)

## Compatibility

- **OS**: Windows 10/11 (64-bit)
- **Game**: Warcraft 3 Reforged

## Known Limitations

- The application needs to find a window titled "Warcraft III" or "Warcraft III: Reforged"
- Chat wheel messages are sent to all chat (no team-only option currently)

## Contributing

Feel free to submit issues and pull requests. Some ideas for improvements:
- Customizable messages
- Team chat option
- Configurable hotkey
- Message categories/pages

## License

This project is open source. Feel free to use and modify as needed.

## Disclaimer

This is a third-party tool and is not affiliated with Blizzard Entertainment.