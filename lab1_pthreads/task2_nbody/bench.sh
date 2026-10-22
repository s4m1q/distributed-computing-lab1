#!/usr/bin/env bash

set -euo pipefail
cd "$(dirname "$0")"
make -s all

N=${1:-1000}
STEPS=${2:-100}
RUNS=${3:-5}
THREADS=${THREADS:-"1 2 4 8"}
INPUT="bodies_$N.txt"
[ -f "$INPUT" ] || python3 gen_bodies.py "$N" > "$INPUT"

stats() { awk '{ for (i = 1; i <= NF; i++) { s += $i; if (i == 1 || $i < m) m = $i }
                 printf "%.6f %.6f\n", m, s / NF }'; }

echo "# nbody: bodies=$N, steps=$STEPS, runs=$RUNS"
echo "version,threads,min_time,avg_time,speedup,efficiency"
for prog in nbody nbody_basic; do
    T1=""
    for t in $THREADS; do

        times=$(for r in $(seq "$RUNS"); do
                    ./$prog "$t" "$STEPS" "$INPUT" 1 0 | awk '/Elapsed time/ {print $4}' || exit 1
                done | tr '\n' ' ')
        read -r min avg <<< "$(echo "$times" | stats)"
        [ -z "$T1" ] && T1=$min
        awk -v p="$prog" -v t="$t" -v min="$min" -v avg="$avg" -v t1="$T1" \
            'BEGIN { s = t1 / min; printf "%s,%d,%.6f,%.6f,%.2f,%.2f\n", p, t, min, avg, s, s / t }'
    done
done
