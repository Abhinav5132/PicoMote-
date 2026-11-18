"""Follow Pico SDK USB serial output across disconnects on Linux."""

import errno
import os
from pathlib import Path
import sys
import termios
import time
import tty


def find_pico():
    for port in sorted(Path("/sys/class/tty").glob("ttyACM*")):
        for device in port.resolve().parents:
            try:
                vendor = (device / "idVendor").read_text().strip()
                product = (device / "idProduct").read_text().strip()
            except FileNotFoundError:
                continue
            if vendor == "2e8a" and product in {"0009", "000a"}:
                return Path("/dev") / port.name
            break
    return None


def listen(port):
    fd = os.open(port, os.O_RDONLY | os.O_NOCTTY)
    try:
        tty.setraw(fd, termios.TCSANOW)
        settings = termios.tcgetattr(fd)
        settings[2] |= termios.CLOCAL
        settings[4] = settings[5] = termios.B115200
        termios.tcsetattr(fd, termios.TCSANOW, settings)
        print(f"Listening on {port}", file=sys.stderr)
        while data := os.read(fd, 4096):
            sys.stdout.buffer.write(data)
            sys.stdout.buffer.flush()
    finally:
        os.close(fd)


def main(selected_port):
    print("Waiting for Pico USB serial (Ctrl-C to stop)", file=sys.stderr)
    while True:
        port = selected_port or find_pico()
        if port:
            try:
                listen(port)
            except OSError as error:
                if error.errno not in {errno.ENOENT, errno.ENODEV, errno.ENXIO, errno.EIO}:
                    raise
            except termios.error as error:
                if error.args[0] not in {errno.ENODEV, errno.ENXIO, errno.EIO}:
                    raise
        time.sleep(0.25)


if __name__ == "__main__":
    try:
        main(sys.argv[1] if len(sys.argv) > 1 else "")
    except KeyboardInterrupt:
        pass
