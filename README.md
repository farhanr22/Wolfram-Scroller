
# Wolfram CA Scroller


A simple, interactive Wolfram Cellular Automata [[1]](https://mathworld.wolfram.com/ElementaryCellularAutomaton.html) [[2]](https://en.wikipedia.org/wiki/Elementary_cellular_automaton) viewer written in C. Created to explore graphics programming, tinker with cellular automata and to learn to set up a cross platform build system.


<p align="center">
  <img src="screenshot.png" alt="A side-by-side view of the Wolfram CA Scroller app and its help screen" width="800"/>
</p>


## Download

Binaries for Windows and Linux are available for download on the [Releases](https://github.com/farhanr22/Wolfram-Scroller/releases) page, and below :
- [Windows `.exe`](https://github.com/farhanr22/Wolfram-Scroller/releases/latest/download/wscroller.exe)
- [Linux executable](https://github.com/farhanr22/Wolfram-Scroller/releases/latest/download/wscroller)

> [!IMPORTANT]
> Your browser or Windows might flag the download as an unrecognized app. This is a standard false positive for new, unsigned programs. To run it, you may need to click "Keep" in your browser, and then **"More info" -> "Run anyway"** on the Windows prompt. Alternatively, you can build the project from source yourself — instructions are below.

## Features

* View any of the 256 elementary Wolfram cellular automata rules, scrolling infinitely.
* Pause/resume the simulation.
* Interactively change the rule, grid size, and color scheme.
* Start from a single middle cell or a randomized first generation.
* Export the current view as a PNG image.
* Start off the simulation with a specific rule by passing it as a command line argument (for eg. `./wscroller 126` on Linux or `wscroller.exe 126` on Windows).


## Controls

| Key / Combination   | Action                                         |
|---------------------|------------------------------------------------|
| **Up / Down Arrows** | Change the rule by 1                          |
| **Shift + Up/Down** | Change the rule by 10                         |
| **Space**           | Pause or resume the simulation                |
| **S**               | Cycle through grid sizes (Small, Medium, Large) |
| **C**               | Cycle through color themes                    |
| **M**               | Reset the grid to a single "middle" cell       |
| **R**               | Reset the grid with a randomized first line    |
| **E**               | Export the current grid view to a `.png` file  |
| **H**               | Show/hide the help screen                      |
| **Esc**             | Close the help screen or quit the application  |

## Building from Source


The system dependencies required are standard C graphics development tools and libraries.  
The graphics library and font are included directly in the source.

**Dependencies on Debian / Ubuntu :**
```bash
sudo apt install build-essential libgl1-mesa-dev libglu1-mesa-dev libx11-dev
```

**Windows :** Has been tested with [**w64devkit**](https://github.com/skeeto/w64devkit), you may use any suitable C compiler suite. Make sure the compiler's `bin` folder is added to your `PATH`.

**Build :** Once the dependencies are installed, run `make` in the project directory. This will create the executable file named `wscroller` (or `wscroller.exe` on Windows).

The font is embedded directly into the executable to avoid external dependencies. This is done by converting the [`font/font.png`](font/font.png) file into a C header file ([`font/font.h`](font/font.h)). It can be (re)generated using the included [Python script](font/convert.py).

## Implementation Details

* The code is split into two main files. `main.c` handles the TIGR window/graphics, user input, and the overall application state. `utils.c` contains the actual cellular automata logic, like calculating the next generation of cells and managing the grid's memory.

* **Memory Allocation :** To avoid large memory allocations when the grid size changes, the memory for the cell grid is allocated for the largest size (`240x240`) only once when the program starts. The small and medium sizes use a portion of this bigger grid, so there's no need to re-allocate memory every time you press `S`.

* **Scrolling Logic :** The infinite scroll effect is split into two phases. Initially, data grid is filled with new rows from top to bottom. Once full, it switches to a more efficient **ring buffer** approach. Instead of shifting every single row up (which would be wasteful), the oldest row in the buffer is continuously overwritten by the new generation. The drawing function is aware of the position of this "wraparound" point and always renders the rows in the correct visual order, making the scroll appear seamless.



## Credits

This project is built on top of a delightful graphics library, [**TIGR**](https://github.com/erkkah/tigr).

The font used is [**Spleen**](https://github.com/fcambus/spleen), specifically the `6x12` size. 

## License

This project is licensed under the [MIT License](LICENSE).




