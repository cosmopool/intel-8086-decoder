#!/bin/sh

./build.sh

for LISTING_ASM in listing_*.asm; do
  LISTING="${LISTING_ASM%.*}"
  printf "testing: %s: " "$LISTING"

  # compile binary for this listing if does not exists
  if [ ! -e "$LISTING" ]; then
    nasm "$LISTING"
  fi

  ./decoder "$LISTING" > "decoded_$LISTING_ASM"
  nasm "decoded_$LISTING_ASM"

  if  diff "$LISTING" "decoded_$LISTING" 1> /dev/null; then 
    printf "pass\n"
  else
    printf "failed\n"
    exit 1
  fi
done
