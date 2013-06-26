.PHONY: clean all
all: fprint_upek1001 extract_cmds write_ppm

fprint_upek1001: fprint_upek1001.c
	gcc $^ -o $@ `pkg-config --cflags --libs libusb-1.0`

extract_cmds: extract_cmds.c
	gcc $^ -o $@

write_ppm: write_ppm.c
	gcc $^ -o $@

clean:
	rm -f fprint_upek1001 extract_cmds
