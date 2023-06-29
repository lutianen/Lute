ALL:
	make release
	make format

format:
	./format_count.sh

debug:
	make clean
	cmake -DCMAKE_BUILD_TYPE=Debug -B build 
	cmake --build build

release:
	make clean
	cmake -DCMAKE_BUILD_TYPE=Release -B build 
	cmake --build build

clean:
	rm -rf build

