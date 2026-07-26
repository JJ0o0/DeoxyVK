#!/bin/bash
set -euo pipefail

case "${1:-debug}" in
    debug)
        build_type="Debug"
        ;;

    release)
        build_type="Release"
        ;;

    *)
        echo "Uso: $0 [debug|release]"
        exit 1
        ;;
esac

case "${2:-sandbox}" in
    sandbox)
        build_sandbox="ON"
        ;;

    no-sandbox)
        build_sandbox="OFF"
        ;;

    *)
        echo "Uso: $0 [debug|release] [sandbox|no-sandbox]"
        exit 1
        ;;
esac

cmake -G Ninja -S . -B build -DCMAKE_BUILD_TYPE="$build_type" -DDEOXY_BUILD_SANDBOX="$build_sandbox"
cmake --build build

if [[ "$build_sandbox" == "ON" ]]; then
    sandbox_executable="./build/bin/Sandbox"

    if [[ -x "$sandbox_executable" ]]; then
        clear
        "$sandbox_executable"
    else
        echo "Error: Sandbox executable not found!"
        exit 1
    fi
fi
