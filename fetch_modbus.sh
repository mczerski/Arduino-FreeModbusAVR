#!/usr/bin/env sh

cd $(dirname $0)
cp freemodbus/modbus/functions/* src/
cp freemodbus/modbus/include/* src/
for f in freemodbus/modbus/rtu/*; do
    if [ "$(basename $f)" != "mbcrc.c" ]; then
        cp $f src/
    fi
done
cp freemodbus/modbus/mb.c src/
