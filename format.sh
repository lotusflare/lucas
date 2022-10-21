#!/bin/sh -e

find src include -iname *.h -o -iname *.c | xargs clang-format -i

# npm install --global prettier @prettier/plugin-lua
find integration -iname *.lua | xargs prettier --loglevel=silent -w
