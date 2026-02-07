#!/bin/sh

cc -o decoder src/decoder.c \
  -g -Wall -Wextra -Werror -Wpointer-arith -Wcast-align -Wunreachable-code
