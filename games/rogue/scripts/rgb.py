#!/usr/bin/env python3

import sys

if __name__ == "__main__":
	r = int(sys.argv[1])
	g = int(sys.argv[2])
	b = int(sys.argv[3])

	print(f"#{r:02x}{g:02x}{b:02x}")
	print("")
	print(f"cxxg::types::RgbColor{{ {r}, {g}, {b} }}")
	print("")
	for _ in range(0, 10):
		print(f"\033[38;2;{r};{g};{b}mHello, World! ###### ABCDE ##### 12345690 \033[0m")
