ALL:
	make release
	make format

run:
	./build/bin/httpServer \
		LutePolaris \
		/home/lux/githubWorkplace/Lute/app/HTML \
		5836 \
		4 \
		127.0.0.1 \
		3306 \
		lutianen \
		lutianen \
		LuxDatabase

format:
	./format_count.sh

debug:
	make clean
	cmake -DCMAKE_BUILD_TYPE=Debug -B build 
	cmake --build build --parallel 8

release:
	make clean
	cmake -DCMAKE_BUILD_TYPE=Release -B build 
	cmake --build build --parallel 8

clean:
	rm -rf build

