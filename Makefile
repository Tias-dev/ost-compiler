all:
	mkdir -p build
	cmake -S . -B ./build -DCMAKE_EXPORT_COMPILE_COMMANDS=1
	cmake --build ./build

run:
	./build/main
