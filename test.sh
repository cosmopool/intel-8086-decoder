#!/bin/sh

./build.sh

RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
NORMAL=$(tput sgr0)

for LISTING_ASM in listing_*.asm; do
  LISTING="${LISTING_ASM%.*}"
  printf "testing: %s: " "$LISTING"

  # compile binary for this listing if does not exists
  if [ ! -e "$LISTING" ]; then
    nasm "$LISTING"
  fi

  if ! ./decoder "$LISTING" > "decoded_$LISTING_ASM"; then
    printf "%s\n" "${RED}decoding failed${NORMAL}"
    exit 1
  fi

  if ! nasm "decoded_$LISTING_ASM"; then
    printf "%s\n" "${RED}compiling failed${NORMAL}"
    exit 1
  fi


  if  diff "$LISTING" "decoded_$LISTING" 1> /dev/null; then 
    printf "%s\n" "${GREEN}pass${NORMAL}"
  else
    printf "%s\n" "${RED}failed${NORMAL}"
    exit 1
  fi
done
