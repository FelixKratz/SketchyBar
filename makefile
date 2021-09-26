FRAMEWORK_PATH = -F/System/Library/PrivateFrameworks
FRAMEWORK      = -framework Carbon -framework Cocoa -framework SkyLight 
BUILD_FLAGS    = -std=c99 -Wall -DNDEBUG -Ofast -fvisibility=hidden
BUILD_PATH     = ./bin
SKETCHYBAR_SRC = ./src/manifest.m
BINS           = $(BUILD_PATH)/sketchybar

.PHONY: all clean install

all: clean $(BINS)

install: clean $(BINS)
	ln ./bin/sketchybar /usr/local/bin/sketchybar
	echo "Install complete... Do not forget to setup the configuration file."

uninstall: clean
	rm /usr/local/bin/sketchybar

debug: BUILD_FLAGS=-std=c99 -Wall -DDEBUG -fsanitize=address -fsanitize=undefined -g -O0 -fvisibility=hidden
debug: clean $(BINS)

update: clean $(BINS)
	rm /usr/local/bin/sketchybar
	ln ./bin/sketchybar /usr/local/bin/sketchybar
	echo "Update complete... ~/.config/ folder not touched and might need update too...."
	
stats: BUILD_FLAGS=-std=c99 -Wall -DSTATS -DNDEBUG -O2 -fvisibility=hidden
stats: clean $(BINS)
	
clean:
	rm -rf $(BUILD_PATH)

$(BUILD_PATH)/sketchybar_x86: $(SKETCHYBAR_SRC)
	mkdir -p $(BUILD_PATH)
	clang $^ $(BUILD_FLAGS) -target x86_64-apple-macos10.13 $(FRAMEWORK_PATH) $(FRAMEWORK) -o $@

$(BUILD_PATH)/sketchybar_arm: $(SKETCHYBAR_SRC)
	mkdir -p $(BUILD_PATH)
	clang $^ $(BUILD_FLAGS) -target arm64-apple-macos11 $(FRAMEWORK_PATH) $(FRAMEWORK) -o $@

$(BUILD_PATH)/sketchybar: $(BUILD_PATH)/sketchybar_arm $(BUILD_PATH)/sketchybar_x86
	lipo -create -output $(BUILD_PATH)/sketchybar $(BUILD_PATH)/sketchybar_x86 $(BUILD_PATH)/sketchybar_arm
