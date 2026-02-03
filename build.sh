#!/usr/bin/sh

cc -o decoder decoder.c \
  -g -Wall -Wextra -Werror -Wpointer-arith -Wcast-align -Wunreachable-code
