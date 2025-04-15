all:
	mkdir -p build
	cmake -S . -B ./build -DCMAKE_EXPORT_COMPILE_COMMANDS=1
	mv ./build/compile_commands.json .
	cmake --build ./build

verbose:
	mkdir -p build
	cmake -S . -B ./build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 --log-level=VERBOSE
	mv ./build/compile_commands.json .
	cmake --build  ./build --verbose

