#!/usr/bin/env bash

if [ $# -ne 2 ]; then
  echo -e "Wrong number of arguments\nUsage:\n\t$0 <source elf file> <destination bin file>"
  exit 1
fi

avr-objcopy -O binary $1 $2
