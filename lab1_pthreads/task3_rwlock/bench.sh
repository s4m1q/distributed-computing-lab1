#!/usr/bin/env bash

set -euo pipefail
cd "$(dirname "$0")"
make -s all

KEYS=${1:-1000}
OPS=${2:-100000}
RUNS=${3:-5}
THREADS=${THREADS:-"1 2 4 8"}
MIXES=${MIXES:-"0.999:0.0005 0.8:0.1"}

stats() { awk '{ for (i = 1; i <= NF; i++) { s += $i; if (i == 1 || $i < m) m = $i }
                 printf "%.6f %.6f\n", m, s / NF }'; }

echo "# list: keys=$KEYS, ops=$OPS, runs=$RUNS"
echo "lock,search,insert,threads,min_time,avg_time"
for mix in $MIXES; do
    search=${mix%%:*}
    insert=${mix##*:}
    for prog in ll_lib ll_my; do
        for t in $THREADS; do

            times=$(for r in $(seq "$RUNS"); do
                        printf "%s\n%s\n%s\n%s\n" "$KEYS" "$OPS" "$search" "$insert" \
                            | ./$prog "$t" | awk '/Elapsed time/ {print $4}' || exit 1
                    done | tr '\n' ' ')
            read -r min avg <<< "$(echo "$times" | stats)"
            echo "$prog,$search,$insert,$t,$min,$avg"
        done
    done
done
