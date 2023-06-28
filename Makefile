debug:
	cmake -DCMAKE_BUILD_TYPE=Debug -B build 
	cmake --build build --parallel 8

release:
	cmake -DCMAKE_BUILD_TYPE=Release -B build 
	cmake --build build --parallel 8

clean:
	rm -rf build

