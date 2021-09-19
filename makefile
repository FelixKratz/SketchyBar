FRAMEWORK_PATH = -F/System/Library/PrivateFrameworks
FRAMEWORK      = -framework Carbon -framework Cocoa -framework SkyLight 
BUILD_FLAGS    = -std=c99 -Wall -DNDEBUG -O0 -fvisibility=hidden -mmacosx-version-min=10.13
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

debug: BUILD_FLAGS=-std=c99 -Wall -DDEBUG -fsanitize=address -fsanitize=undefined -g -O0 -fvisibility=hidden -mmacosx-version-min=10.13
debug: clean $(BINS)

update: clean $(BINS)
	rm /usr/local/bin/sketchybar
	ln ./bin/sketchybar /usr/local/bin/sketchybar
	echo "Update complete... ~/.config/ folder not touched and might need update too...."
	
stats: BUILD_FLAGS=-std=c99 -Wall -DSTATS -DNDEBUG -O2 -fvisibility=hidden -mmacosx-version-min=10.13
stats: clean $(BINS)
	
clean:
	rm -rf $(BUILD_PATH)

$(BUILD_PATH)/sketchybar: $(SKETCHYBAR_SRC)
	mkdir -p $(BUILD_PATH)
	clang $^ $(BUILD_FLAGS) $(FRAMEWORK_PATH) $(FRAMEWORK) -o $@
