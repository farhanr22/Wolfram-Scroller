# Usage : font.py font.png out.h
# python png.py font.png font.h
import sys


def read_png_as_bytes(file_path):
    with open(file_path, "rb") as file:
        png_data = file.read()
    return [byte for byte in png_data]


file_path = sys.argv[1]
out_path = sys.argv[2]

with open(file_path, "rb") as file:
    png_data = file.read()
    png_bytes = [byte for byte in png_data]

bytes_str = "unsigned char png_data[] = {"
for byte in png_bytes:
    bytes_str += f"0x{byte:02x},"
bytes_str += "};"


with open(out_path, "w") as file:
    file.write(bytes_str)
