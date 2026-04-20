# main:
# # 	clang++ *.cpp -I./ -I./SDL2-2.28.5/ -I./SDL2_image/ -I./SDL2_ttf -I./SDL2_mixer/ \
# # 	-lSDL2_image -lSDL2 -lSDL2_ttf -lSDL2_mixer -O3 -std=c++17 -Wno-deprecated-declarations -o game_engine_linux
# 	clang++ *.cpp $(sdl2-config --cflags --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -O3 -std=c++17 -Wno-deprecated-declarations -o game_engine_linux

# main:
# 	clang++ *.cpp -I/usr/include/SDL2 -O3 -std=c++17 -Wno-deprecated-declarations -L/usr/local/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -o game_engine_linux


# main:
# 	clang++ *.cpp Lua/*.c -I/usr/include/SDL2 -ILua -ILuaBridge/Source -O3 -std=c++17 -Wno-deprecated-declarations -L/usr/local/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -o game_engine_linux


# main:
# 	clang++ *.cpp Lua/*.c -I/usr/include/SDL2 -ILua -I. -O3 -std=c++17 -Wno-deprecated-declarations -Wno-deprecated -L/usr/local/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -o game_engine_linux
# Build liblua.a first, then the engine
main: liblua
	clang++ *.cpp box2d/src/**/*.cpp -I/usr/include/SDL2 -ILua -I. -Ibox2d/include -Ibox2d/src -O3 -std=c++17 \
	-Wno-deprecated-declarations -Wno-deprecated \
	-LLua -L/usr/local/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer \
	-llua -o game_engine_linux

liblua:
	cd Lua && clang -O3 -Wno-deprecated-declarations -c *.c && ar rcs liblua.a *.o