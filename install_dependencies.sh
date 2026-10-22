#!/usr/bin/env bash

set -euo pipefail

usage() {
    cat <<'HELP'
Использование: bash install_dependencies.sh [--check | --help]

Без параметров: обновить список пакетов и установить зависимости через apt.
  --check       Проверить окружение без установки пакетов и без sudo.
  --help        Показать эту справку.
HELP
}

if (( $# > 1 )); then
    usage >&2
    exit 2
fi

mode=install
case "${1:-}" in
    '') ;;
    --check) mode=check ;;
    --help|-h) usage; exit 0 ;;
    *) usage >&2; exit 2 ;;
esac

if [[ ! -r /etc/os-release ]]; then
    printf 'Ошибка: запускайте скрипт в Ubuntu 24.04 LTS.\n' >&2
    exit 1
fi

source /etc/os-release
if [[ "${ID:-}" != ubuntu || "${VERSION_ID:-}" != 24.04 ]]; then
    printf 'Ошибка: нужна Ubuntu 24.04 LTS; обнаружена %s.\n' "${PRETTY_NAME:-неизвестная ОС}" >&2
    exit 1
fi

check_dependencies() {
    local failed=0 tool
    for tool in gcc make python3 git awk lscpu free nproc tee lsb_release; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            printf 'Не найдено: %s\n' "$tool" >&2
            failed=1
        fi
    done

    if command -v gcc >/dev/null 2>&1; then
        if ! printf '%s\n' '#include <pthread.h>' '#include <math.h>' \
            '#include <sys/time.h>' 'int main(void) { return 0; }' \
            | gcc -pthread -x c -fsyntax-only - >/dev/null 2>&1; then
            printf 'Не найдены рабочие заголовки C/pthreads; нужен build-essential.\n' >&2
            failed=1
        fi
    fi

    if [[ -x /usr/bin/python3 ]]; then
        if ! /usr/bin/python3 -c \
            'import matplotlib; matplotlib.use("Agg"); import matplotlib.pyplot' \
            >/dev/null 2>&1; then
            printf 'Не найден рабочий Matplotlib для /usr/bin/python3.\n' >&2
            failed=1
        fi
    fi

    if (( failed != 0 )); then
        printf 'Для установки запустите: bash install_dependencies.sh\n' >&2
        return 1
    fi
    printf 'Все зависимости лабораторной доступны.\n'
}

if [[ "$mode" == check ]]; then
    check_dependencies
    exit 0
fi

packages=(
    build-essential
    python3
    python3-matplotlib
    git
    ca-certificates
    coreutils
    mawk
    util-linux
    procps
    lsb-release
)

elevate=()
if (( EUID != 0 )); then
    if ! command -v sudo >/dev/null 2>&1; then
        printf 'Ошибка: нужен sudo или запуск от root.\n' >&2
        exit 1
    fi
    elevate=(sudo)
fi

printf 'Установка зависимостей лабораторной для Ubuntu 24.04 LTS...\n'
"${elevate[@]}" apt-get update
"${elevate[@]}" apt-get install --yes --no-install-recommends "${packages[@]}"

check_dependencies
printf 'GCC: %s\n' "$(gcc -dumpfullversion)"
/usr/bin/python3 -c \
    'import sys, matplotlib; print("Python:", sys.version.split()[0]); print("Matplotlib:", matplotlib.__version__)'
