# Build the project for Pico 2 (RP2350)
build:
    cmake -B build -S .
    cmake --build build -j

# Flash using picotool
flash: build
    picotool load -fx build/remote-pi.uf2

# Clean build directory
clean:
    rm -rf build

# Follow the Pico USB serial port. Optionally pass a device path.
[positional-arguments]
listen port="":
    sudo python3 scripts/listen.py "$1"
