# all:
# 	gcc -shared -fPIC -o test_plugin2.so -I ../../include/ -L../../build/ test_plugin.c

CC = gcc
CFLAGS = -Wall -fPIC -I../../include $(shell pkg-config --cflags gtk4)
LIBS = $(shell pkg-config --libs gtk4)

all:
	$(CC) -shared $(CFLAGS) -o test_plugin2.so test_plugin.c $(LIBS)
