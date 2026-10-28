# Source-seam test for lazy item-window allocation.
#
#   make test        compile and run
#   make test-asan   same, under AddressSanitizer and UndefinedBehaviorSanitizer
#   make clean       remove both build directories
#
# The test links the bar sources directly and drives the production callers in
# process. It opens real windows far offscreen and closes them again; it never
# starts the mach server, reads a config, activates a panel or touches a running
# bar. It asserts which items own a window, not anything about pixels, latency
# or presentation. Sanitizer builds use their own object and binary directories,
# so a sanitized run can never reuse objects from a plain one.
SRC ?= ../src
ODIR ?= obj
BIN ?= bin
CFLAGS ?= -std=c99 -Wall -g -O0 -fvisibility=hidden -fno-common \
          -target arm64-apple-macos11 -I$(SRC)
LIBS = -framework Carbon \
       -framework AppKit \
       -framework QuartzCore \
       -framework CoreAudio \
       -framework CoreWLAN \
       -framework CoreVideo \
       -framework IOKit \
       -F/System/Library/PrivateFrameworks \
       -framework SkyLight \
       -framework DisplayServices \
       -framework MediaRemote

_OBJ = alias.o background.o bar_item.o custom_events.o event.o graph.o \
       image.o mouse.o shadow.o font.o text.o message.o bar.o color.o \
       window.o bar_manager.o display.o display_nsscreen.om group.o mach.o \
       popup.o animation.o workspace.om volume.o slider.o power.o wifi.om \
       media.om hotload.o app_windows.o layer.om surface.o context.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ))

.PHONY: test test-asan clean
test: $(BIN)/lazy-item-windows-test
	$(BIN)/lazy-item-windows-test

# Separate directories, so this never links objects built without the sanitizers.
test-asan:
	$(MAKE) ODIR=obj-asan BIN=bin-asan \
	        CFLAGS="$(CFLAGS) -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer" test

$(BIN)/lazy-item-windows-test: $(ODIR)/lazy_item_windows_test.o $(OBJ) | $(BIN)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(ODIR)/lazy_item_windows_test.o: lazy_item_windows_test.c | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(SRC)/%.c | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.om: $(SRC)/%.m | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR) $(BIN):
	mkdir -p $@

clean:
	rm -rf $(ODIR) $(BIN) obj-asan bin-asan
