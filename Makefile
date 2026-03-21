all:
	mkdir -p build
	cmake -S . -B ./build -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_EXPORT_COMPILE_COMMANDS=1
	mv ./build/compile_commands.json .
	cmake --build ./build -- -j4

release debug: %: cmake-%
	cmake --build build-$@ -- -j4

cmake-debug: build-debug
	cmake -S . -B $< -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_EXPORT_COMPILE_COMMANDS=1
	cp ./build-debug/compile_commands.json ./

cmake-release: build-release
		cmake -S . -B $< -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_EXPORT_COMPILE_COMMANDS=1
	cp ./build-release/compile_commands.json ./

build-debug build-release:
	mkdir $@

~/.local/bin:
	mkdir -p $@

install: release ~/.local/bin
	cp ./build-release/{ost,ostbuild,ostdb,osttest,tu4run} ~/.local/bin/


verbose:
	mkdir -p build
	cmake -S . -B ./build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 --log-level=VERBOSE
	mv ./build/compile_commands.json .
	cmake --build  ./build --verbose -- -j4

