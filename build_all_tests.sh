#!/usr/bin/env bash
# Check buildability of each firmware test and print a compact summary.
# Status meanings (these are build results, NOT hardware test results):
#   PASS           - compilation and linking succeeded.
#   FAIL           - build or CMake configuration failed; see the log.
#                   Also includes tests missing from root CMakeLists.txt.
set -u

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT/build/all-tests"
LOG_DIR="$ROOT/build/all-tests-logs"

if [[ ! -f "$ROOT/CMakeLists.txt" || ! -d "$ROOT/test" ]]; then
    echo "ERROR: run the script from a copy placed in the repository root." >&2
    exit 2
fi

for tool in cmake ninja arm-none-eabi-gcc; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "ERROR: missing required tool: $tool" >&2
        exit 2
    fi
done

mkdir -p "$LOG_DIR"
cd "$ROOT" || exit 2

count=0
passed=0
failed=0

printf '%-29s %-16s %s\n' 'TEST' 'RESULT' 'LOG'
printf '%s\n' '--------------------------------------------------------------------------------'

shopt -s nullglob
for dir in "$ROOT"/test/*/; do
    [[ -f "$dir/CMakeLists.txt" ]] || continue

    name="$(basename "$dir")"
    log="$LOG_DIR/$name.log"
    log_relative="${log#$ROOT/}"
    ((count+=1))

    # Modules not listed in the root CMakeLists.txt cannot be built via TOP.
    if ! grep -Eq "^[[:space:]]*add_subdirectory[[:space:]]*\([[:space:]]*\"?test/${name}\"?([[:space:]]|\))" "$ROOT/CMakeLists.txt"; then
        printf 'Test %s is not included in the root CMakeLists.txt\n' "$name" > "$log"
        ((failed+=1))
        printf '%-29s %-16s %s\n' "$name" 'FAIL' "$log_relative"
        continue
    fi

    target="$(sed -nE 's/^[[:space:]]*project[[:space:]]*\([[:space:]]*"?([[:alnum:]_.+-]+)"?.*/\1/p' "$dir/CMakeLists.txt" | head -n 1)"
    if [[ -z "$target" ]]; then
        printf 'No project(...) name in test/%s/CMakeLists.txt\n' "$name" > "$log"
        ((failed+=1))
        printf '%-29s %-16s %s\n' "$name" 'FAIL' "$log_relative"
        continue
    fi

    # Share compiled dependencies between tests, but relink main.elf for each TOP.
    if {
        cmake -S "$ROOT" -B "$BUILD_DIR" -G Ninja -DTOP="$target" &&
        cmake --build "$BUILD_DIR" --target main.elf
    } > "$log" 2>&1; then
        ((passed+=1))
        printf '%-29s %-16s %s\n' "$name" 'PASS' "$log_relative"
    else
        ((failed+=1))
        printf '%-29s %-16s %s\n' "$name" 'FAIL' "$log_relative"
    fi
done
shopt -u nullglob

if (( count == 0 )); then
    echo "ERROR: no test modules found in test/." >&2
    exit 2
fi

printf '\nPASS: %d | FAIL: %d\n' "$passed" "$failed"
printf '%s\n' 'PASS — сборка успешна | FAIL — ошибка (причина в LOG)'
(( failed == 0 ))

