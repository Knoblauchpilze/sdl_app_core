# sdl_app_core
Minimal multi-threaded application with sdl 2.0

Contains base classes to make a graphic application with SDL 2.0, supporting a multithreading environment for rendering and event handling, so that it can be easily extended with custom graphic containers.


# Installation

- Clone the repo: `git clone git@github.com:Knoblauchpilze/sdl_app_core.git`
- Go to the project's directory `cd ~/path/to/the/repo`
- Compile: `make r`
- Install: `make install`

# Usage

Don't forget to add `/usr/local/lib` to your `LD_LIBRARY_PATH` to be able to load shared libraries at runtime.
