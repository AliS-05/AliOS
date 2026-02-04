all:
	$(MAKE) -C build

run:
	$(MAKE) -C build run

debug:
	$(MAKE) -C build debug

clean:
	$(MAKE) -C build clean

.PHONY: all run debug clean
