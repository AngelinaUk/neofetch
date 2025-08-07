# Neofetch for Windows

A fast, lightweight Windows implementation of neofetch written in C++. This system information tool displays your system configuration in a visually appealing way using ASCII art and colorful output.

## Features

- **Native Windows Implementation**: Uses Windows APIs and WMI for accurate system information
- **Colorful ASCII Art**: Beautiful Windows logo with customizable color themes
- **Comprehensive System Info**: Displays detailed information about your system
- **Multiple Color Themes**: Choose from 7 different color schemes
- **Fast Performance**: Written in C++ for optimal speed
- **ANSI Color Support**: Automatically enables color support in Windows console

## System Information Displayed

- **User & Host**: Current username and computer name
- **Operating System**: Windows version, build number
- **Kernel**: NT kernel version
- **Uptime**: System uptime in days, hours, and minutes
- **Shell**: Default command shell (usually cmd.exe)
- **Terminal**: Detected terminal emulator
- **CPU**: Processor information (cleaned and formatted)
- **GPU**: Graphics card information
- **Memory**: RAM usage (used/total in GB)
- **Disk**: Storage usage for C: drive
- **Resolution**: Primary display resolution
- **Theme**: Windows theme (Light/Dark)
- **Local IP**: Local network IP address
- **Color Palette**: ANSI color demonstration

## Requirements

- Windows 10/11 (or Windows with WMI support)
- Visual Studio 2019+ or compatible C++ compiler
- Windows SDK

## Building

### Using Visual Studio

1. Clone or download the source code
2. Open Visual Studio and create a new Console App project
3. Replace the default code with the provided source
4. Build the project (Ctrl+Shift+B)

### Using Command Line (with MSVC)

```bash
cl /EHsc neofetch.cpp /link wbemuuid.lib psapi.lib version.lib shell32.lib iphlpapi.lib
```

### Using MinGW-w64

```bash
g++ -std=c++11 neofetch.cpp -lwbemuuid -lpsapi -lversion -lshell32 -liphlpapi -o neofetch.exe
```

## Usage

### Basic Usage

```bash
neofetch.exe
```

### Color Themes

Choose from different color themes using command line flags:

```bash
neofetch.exe -blue      # Blue theme (default)
neofetch.exe -red       # Red theme
neofetch.exe -green     # Green theme
neofetch.exe -yellow    # Yellow theme
neofetch.exe -magenta   # Magenta theme
neofetch.exe -cyan      # Cyan theme
neofetch.exe -white     # White theme
```

### Help and Version

```bash
neofetch.exe --help     # Show help message
neofetch.exe -h         # Show help message
neofetch.exe --version  # Show version information
neofetch.exe -v         # Show version information
```

## Example Output

```
                                ..,                  john@DESKTOP-ABC123
                    ....,,:;+ccllll                  --------------------
      ...,,+:;  cllllllllllllllllll                  OS: Windows 11 Pro 10.0.22621 (Build 22621)
,cclllllllllll  lllllllllllllllllll                  Host: DESKTOP-ABC123
llllllllllllll  lllllllllllllllllll                  Kernel: NT 10.0.22621
llllllllllllll  lllllllllllllllllll                  Uptime: 2 days, 5 hours, 23 mins
llllllllllllll  lllllllllllllllllll                  Shell: cmd.exe
llllllllllllll  lllllllllllllllllll                  Terminal: Windows Terminal
llllllllllllll  lllllllllllllllllll                  CPU: Intel Core i7-9700K @ 3.60GHz
                                                      GPU: NVIDIA GeForce RTX 3070
llllllllllllll  lllllllllllllllllll                  Memory: 12.3GB / 32.0GB
llllllllllllll  lllllllllllllllllll                  Disk: 245.7GB / 931.5GB (C:)
llllllllllllll  lllllllllllllllllll                  Resolution: 2560x1440
llllllllllllll  lllllllllllllllllll                  Theme: Dark
llllllllllllll  lllllllllllllllllll                  Local IP: 192.168.1.100
`'ccllllllllll  lllllllllllllllllll                  
       `' \*::  :ccllllllllllllllll                  ███████████████████████████████
                       ````''*::cll                  ███████████████████████████████
                                 ``                  
```

## Technical Details

### Architecture

- **WMI Integration**: Uses Windows Management Instrumentation for system queries
- **Win32 APIs**: Leverages native Windows APIs for performance and accuracy
- **ANSI Support**: Automatically enables virtual terminal processing for colors
- **Error Handling**: Robust error handling for WMI and API failures

### Dependencies

- `wbemuuid.lib` - WMI support
- `psapi.lib` - Process and memory information
- `version.lib` - Version information APIs
- `shell32.lib` - Shell integration
- `iphlpapi.lib` - Network adapter information

### Key Classes

- **WMIQuery**: Handles WMI connections and queries
- **Neofetch**: Main application class with system info gathering
- **ColorTheme**: Color scheme management

## Customization

### Adding New Themes

Modify the `setTheme()` method in the `Neofetch` class to add new color combinations:

```cpp
} else if (themeName == "custom") {
    theme = {"custom", "\033[38;5;208m", "\033[1;38;5;208m", "\033[0m", "\033[0m"};
}
```

### Modifying Information Display

Edit the `display()` method to add, remove, or reorder system information fields.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on different Windows versions
5. Submit a pull request

## Known Issues

- Some virtualized environments may not report all hardware correctly
- Network adapter detection may vary based on system configuration
- WMI queries may be slower on older systems

## License

MIT License

Copyright (c) 2025 Sentinet Community Centre

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Acknowledgments

- Windows ASCII art adapted from various sources
- Community feedback and contributions

---

**Note**: This implementation is specifically designed for Windows systems and uses Windows-specific APIs. 
