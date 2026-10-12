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

listen: 
    sudo minicom -D /dev/ttyACM0 -b 115200