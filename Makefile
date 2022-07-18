.PHONY: setup setup-debug build build-debug run run-debug clean

BUILD_FOLDER_RELEASE = build
BUILD_FOLDER_DEBUG = build_debug

setup:
	meson $(BUILD_FOLDER_RELEASE) --buildtype=release

setup-debug:
	meson $(BUILD_FOLDER_DEBUG) --buildtype=debug

build: setup
	meson compile -C $(BUILD_FOLDER_RELEASE)

build-debug: setup-debug
	meson compile -C $(BUILD_FOLDER_DEBUG)

run: build
	./$(BUILD_FOLDER_RELEASE)/sudoku_solver

run-debug: build-debug
	./$(BUILD_FOLDER_DEBUG)/sudoku_solver

clean:
	rm -rf $(BUILD_FOLDER_RELEASE) $(BUILD_FOLDER_DEBUG)
