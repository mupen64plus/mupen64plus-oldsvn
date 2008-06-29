all: master_server.c test_client.c
	gcc -Wall `sdl-config --cflags` master_server.c -o master_server `sdl-config --libs` -lSDL_net -I.
	gcc -Wall `sdl-config --cflags` test_client.c -o test_client `sdl-config --libs` -lSDL_net -I.

