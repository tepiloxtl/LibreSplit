all:
	gcc -shared -fPIC -o time_graph.so -I../../include/ -I/usr/include/ -I/usr/include/luajit-2.1 -L/usr/lib/ -L/usr/lib64/ time_graph.c `pkg-config --cflags --libs gtk+-3.0`
