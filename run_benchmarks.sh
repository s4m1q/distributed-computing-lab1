#!/usr/bin/env bash

set -euo pipefail
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
LAB_DIR="$SCRIPT_DIR/lab1_pthreads"

if [[ ! -d "$LAB_DIR/task1_mandelbrot" ]]; then
    LAB_DIR="$SCRIPT_DIR"
fi
for task in task1_mandelbrot task2_nbody task3_rwlock; do
    if [[ ! -f "$LAB_DIR/$task/bench.sh" ]]; then
        printf 'Не найден скрипт замеров: %s/%s/bench.sh\n' "$LAB_DIR" "$task" >&2
        exit 1
    fi
done
cd -- "$LAB_DIR"

RUNS=${RUNS:-5}
if [[ ! "$RUNS" =~ ^[1-9][0-9]*$ ]]; then
    printf 'RUNS должен быть положительным целым числом.\n' >&2
    exit 1
fi
CORES=$(nproc)

THREADS=""
T=1
while [ "$T" -lt "$CORES" ]; do THREADS="$THREADS $T"; T=$((T * 2)); done
THREADS="$THREADS $CORES $((CORES * 2))"
export THREADS

OUT="$SCRIPT_DIR/results_$(date +%Y-%m-%d_%H-%M-%S).txt"

{
    echo "=== Система ==="
    date
    lsb_release -ds 2>/dev/null || grep PRETTY_NAME /etc/os-release
    echo "Ядро Linux: $(uname -r)"
    LC_ALL=C lscpu | grep -E '^(Model name|CPU\(s\)|Thread\(s\) per core|Core\(s\) per socket|Socket\(s\)|CPU max MHz|L1d cache|L2 cache|L3 cache):'
    LC_ALL=C free -h | head -2
    gcc --version | head -1
    for ps in /sys/class/power_supply/*; do
        if [ "$(cat "$ps/type" 2>/dev/null)" = "Mains" ] && [ "$(cat "$ps/online" 2>/dev/null)" = "0" ]; then
            echo "ВНИМАНИЕ: ноутбук работает от батареи — поставьте на зарядку, иначе частота процессора занижена"
        fi
    done
    echo "Потоки:$THREADS; повторов каждого запуска: $RUNS"
    echo

    echo "=== Задача 1: Мандельброт, 500000 точек ==="
    bash task1_mandelbrot/bench.sh 500000 "$RUNS"
    echo

    echo "=== Задача 2: N тел, 2000 тел, 100 шагов ==="
    bash task2_nbody/bench.sh 2000 100 "$RUNS"
    echo

    echo "=== Задача 3: список, 99.9% поиск, 1000000 операций ==="
    MIXES="0.999:0.0005" bash task3_rwlock/bench.sh 1000 1000000 "$RUNS"
    echo

    echo "=== Задача 3: список, 80% поиск, 100000 операций ==="
    MIXES="0.8:0.1" bash task3_rwlock/bench.sh 1000 100000 "$RUNS"
    echo

    echo "Готово: $(date)"
} 2>&1 | tee "$OUT"

echo
echo "Результаты сохранены в файл: $OUT"
