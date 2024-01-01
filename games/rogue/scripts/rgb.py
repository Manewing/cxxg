#!/usr/bin/env python3

import sys

def hex_to_rgb(hex: str):
	h = hex.lstrip('#')
	return tuple(int(h[i:i+2], 16) for i in (0, 2, 4))

def rgb_to_hex(r: int, g: int, b: int):
	return f"#{r:02x}{g:02x}{b:02x}"

if __name__ == "__main__":
	if len(sys.argv) != 2 and len(sys.argv) != 4:
		print("Usage: rgb.py <hex> or rgb.py <r> <g> <b>")
		exit(1)
	if len(sys.argv) == 2:
		r, g, b = hex_to_rgb(sys.argv[1])
	else:
		r = int(sys.argv[1])
		g = int(sys.argv[2])
		b = int(sys.argv[3])

	print(rgb_to_hex(r, g, b))
	print("")
	print(f"cxxg::types::RgbColor{{ {r}, {g}, {b} }}")
	print("")
	for _ in range(0, 5):
		print(f"\033[38;2;{r};{g};{b}mHello, World! ###### ABCDE ##### 12345690 \033[0m")
	for _ in range(0, 5):
		print(f"\033[48;2;{r};{g};{b}mHello, World! ###### ABCDE ##### 12345690 \033[0m")
