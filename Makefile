lonely:
	c++ --std=c++11 -lSDL lonely.cpp -o lonely

lonely-sdl2:
	c++ -DUSE_SDL2 --std=c++11 -lSDL2 lonely.cpp -o lonely-sdl2
