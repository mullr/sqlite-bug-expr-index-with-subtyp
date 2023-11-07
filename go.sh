#!/usr/bin/env sh

rm -f repro
gcc repro.c -lsqlite3 -o repro
rm -f fruit.db
./repro
