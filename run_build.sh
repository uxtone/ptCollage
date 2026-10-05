#!/usr/bin/env bash
nix develop --command fish -c 'cmake -B build -S . --toolchain "$WINELIB64_TOOLCHAIN" -DUSE_UNICODE=on -DBUNDLED_PNG=off -DBUNDLE_BUILD_DIR=release && cmake --build build --parallel'
