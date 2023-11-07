#!/usr/bin/env sh

rm -f repro
gcc -Isqlite-amalgamation repro.c sqlite-amalgamation/sqlite3.c -o repro
rm -f fruit.db
./repro
