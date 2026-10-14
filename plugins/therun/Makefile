# all:
# 	gcc -shared -fPIC -o therun_plugin.so -I../../include/ -I/usr/include/ -I/usr/include/luajit-2.1 -L/usr/lib/ -L/usr/lib64/ -lcurl therun.c

CC = gcc
CFLAGS = -Wall -fPIC -I../../include $(shell pkg-config --cflags gtk4)
LIBS = $(shell pkg-config --libs gtk4 libcurl)

all:
	$(CC) -shared $(CFLAGS) -o therun.so therun.c $(LIBS)
