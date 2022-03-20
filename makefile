FRAMEWORK_PATH = -F/System/Library/PrivateFrameworks
FRAMEWORK      = -framework Carbon -framework Cocoa -framework SkyLight 
BUILD_FLAGS    = -std=c99 -Wall -DNDEBUG -Ofast -ffast-math -fvisibility=hidden
BUILD_PATH     = ./bin
SKETCHYBAR_SRC = ./src/manifest.m
UNIVERSAL_BINS = $(BUILD_PATH)/sketchybar
ARM_BINS       = $(BUILD_PATH)/sketchybar_arm
x86_BINS       = $(BUILD_PATH)/sketchybar_x86

.PHONY: all clean install

all: clean $(UNIVERSAL_BINS)

arm: clean $(ARM_BINS)

x86: clean $(x86_BINS)

install: clean $(UNIVERSAL_BINS)
	ln ./bin/sketchybar /usr/local/bin/sketchybar
	echo "Install complete... Do not forget to setup the configuration file."

uninstall: clean
	rm /usr/local/bin/sketchybar

profile: BUILD_FLAGS=-std=c99 -Wall -DDEBUG -g -Ofast -fvisibility=hidden 
profile: clean $(x86_BINS)

leak: BUILD_FLAGS=-std=c99 -Wall -DDEBUG -g
leak: clean $(ARM_BINS)
	/usr/libexec/PlistBuddy -c "Add :com.apple.security.get-task-allow bool true" bin/tmp.entitlements
	codesign -s - --entitlements bin/tmp.entitlements -f ./bin/sketchybar_arm
	leaks -atExit -- ./bin/sketchybar_arm


debug: BUILD_FLAGS=-std=c99 -Wall -DDEBUG -g -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer 
debug: clean $(ARM_BINS)
	./bin/sketchybar_arm

update: clean $(UNIVERSAL_BINS)
	rm /usr/local/bin/sketchybar
	ln ./bin/sketchybar /usr/local/bin/sketchybar
	echo "Update complete... ~/.config/ folder not touched and might need update too...."
	
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
