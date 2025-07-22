CFLAGS   = -std=c99 -Wall -O3 -ffast-math -fvisibility=hidden -fno-common

LIBS     = -framework Carbon \
					 -framework AppKit \
					 -framework CoreAudio \
					 -framework CoreWLAN \
					 -framework CoreVideo \
					 -framework IOKit \
					 -F/System/Library/PrivateFrameworks \
					 -framework SkyLight \
					 -framework DisplayServices \
					 -framework MediaRemote

ODIR     = bin
SRC      = src

_OBJ = alias.o background.o bar_item.o custom_events.o event.o graph.o \
			 image.o mouse.o shadow.o font.o text.o message.o mouse.o bar.o color.o \
			 window.o bar_manager.o display.o group.o mach.o popup.o \
			 animation.o workspace.om volume.o slider.o power.o wifi.om media.om \
			 hotload.o app_windows.o

OBJ  = $(patsubst %, $(ODIR)/%, $(_OBJ))

.PHONY: all clean arm x86 profile leak universal

all: clean universal

leak: CFLAGS=-std=c99 -Wall -g
leak: clean arm64
leak:
	/usr/libexec/PlistBuddy -c "Add :com.apple.security.get-task-allow bool true" bin/tmp.entitlements
	codesign -s - --entitlements bin/tmp.entitlements -f ./bin/sketchybar
	leaks -atExit -- ./bin/sketchybar

x86: CFLAGS+=-target x86_64-apple-macos10.13
x86: $(ODIR)/sketchybar

arm64: CFLAGS+=-target arm64-apple-macos11
arm64: $(ODIR)/sketchybar

universal:
	$(MAKE) x86
	mv $(ODIR)/sketchybar $(ODIR)/sketchybar_x86
	rm -rf $(ODIR)/*.o*
	$(MAKE) arm64
	mv $(ODIR)/sketchybar $(ODIR)/sketchybar_arm64
	lipo -create -output $(ODIR)/sketchybar $(ODIR)/sketchybar_x86 $(ODIR)/sketchybar_arm64

debug: CFLAGS=-std=c99 -Wall -g
debug: arm64

asan: CFLAGS=-std=c99 -Wall -g -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
asan: clean arm64
	./bin/sketchybar

$(ODIR)/sketchybar: $(SRC)/sketchybar.c $(OBJ) | $(ODIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(ODIR)/%.o: $(SRC)/%.c $(SRC)/%.h | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.om: $(SRC)/%.m $(SRC)/%.h | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR):
	mkdir $(ODIR)

clean:
	rm -rf $(ODIR)
