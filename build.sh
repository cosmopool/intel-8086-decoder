#!/bin/sh

echo "building with debug symbols..."
cc -o decoder src/decoder.c \
   -g -DDEBUG \
   -Wall \
   -Wextra \
   -Werror \
   -Wpointer-arith \
   -Wcast-align \
   -Wunreachable-code
