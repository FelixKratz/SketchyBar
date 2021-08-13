FRAMEWORK_PATH = -F/System/Library/PrivateFrameworks
FRAMEWORK      = -framework Carbon -framework Cocoa -framework CoreServices -framework SkyLight -framework ScriptingBridge -framework IOKit
BUILD_FLAGS    = -std=c99 -Wall -DDEBUG -g -O0 -fvisibility=hidden -mmacosx-version-min=10.13
BUILD_PATH     = ./bin
DOC_PATH       = ./doc
SMP_PATH       = ./examples
SKETCHYBAR_SRC   = ./src/manifest.m
BINS           = $(BUILD_PATH)/sketchybar

.PHONY: all clean install man

all: clean $(BINS)

install: BUILD_FLAGS=-std=c99 -Wall -DNDEBUG -O2 -fvisibility=hidden -mmacosx-version-min=10.13
install: clean $(BINS)

stats: BUILD_FLAGS=-std=c99 -Wall -DSTATS -DNDEBUG -O2 -fvisibility=hidden -mmacosx-version-min=10.13
stats: clean $(BINS)
	
clean:
	rm -rf $(BUILD_PATH)

$(BUILD_PATH)/sketchybar: $(SKETCHYBAR_SRC)
	mkdir -p $(BUILD_PATH)
	clang $^ $(BUILD_FLAGS) $(FRAMEWORK_PATH) $(FRAMEWORK) -o $@
