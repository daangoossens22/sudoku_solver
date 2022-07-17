.PHONY: build build-debug run run-debug clean

BUILD_FOLDER_RELEASE = build
BUILD_FOLDER_DEBUG = build_debug

build:
	meson $(BUILD_FOLDER_RELEASE) --buildtype=release
	meson compile -C $(BUILD_FOLDER_RELEASE)

build-debug:
	meson $(BUILD_FOLDER_DEBUG) --buildtype=debug
	meson compile -C $(BUILD_FOLDER_DEBUG)

run: build
	./$(BUILD_FOLDER_RELEASE)/sudoku_solver

run-debug: build-debug
	./$(BUILD_FOLDER_DEBUG)/sudoku_solver

clean:
	rm -rf $(BUILD_FOLDER_RELEASE) $(BUILD_FOLDER_DEBUG)
