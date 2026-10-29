#!/bin/bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build
ln -sf build/compile_commands.json .
