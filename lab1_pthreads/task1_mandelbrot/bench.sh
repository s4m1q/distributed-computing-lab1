#!/usr/bin/env bash

set -euo pipefail
cd "$(dirname "$0")"
make -s mandelbrot

NPOINTS=${1:-500000}
RUNS=${2:-5}
THREADS=${THREADS:-"1 2 4 8"}

stats() { awk '{ for (i = 1; i <= NF; i++) { s += $i; if (i == 1 || $i < m) m = $i }
                 printf "%.6f %.6f\n", m, s / NF }'; }

echo "# mandelbrot: npoints=$NPOINTS, runs=$RUNS"
echo "threads,min_time,avg_time,speedup,efficiency"
T1=""
for t in $THREADS; do
    times=$(for r in $(seq "$RUNS"); do
                ./mandelbrot "$t" "$NPOINTS" /dev/null | awk '/Elapsed time/ {print $4}' || exit 1
            done | tr '\n' ' ')
    read -r min avg <<< "$(echo "$times" | stats)"
    [ -z "$T1" ] && T1=$min
    awk -v t="$t" -v min="$min" -v avg="$avg" -v t1="$T1" \
        'BEGIN { s = t1 / min; printf "%d,%.6f,%.6f,%.2f,%.2f\n", t, min, avg, s, s / t }'
done
