#!/bin/bash
#SBATCH --job-name=mpi_assign_P
#SBATCH -N 1
#SBATCH --ntasks-per-node=16
#SBATCH --partition=cpu
#SBATCH --time=00:05:00
#SBATCH --output=assign_%j.out
#SBATCH --error=assign_%j.err

module load compiler/oneapi-2024/mpi

# ---------------- Compile ----------------
mpicc ass1.c -o ass1 -lm
if [ $? -ne 0 ]; then
    echo "Compilation failed"
    exit 1
fi

# ---------------- Fixed parameters ----------------
D1=2
D2=4
T=10
SEED=1000

# ---------------- Process count ----------------
P=$1
if [ -z "$P" ]; then
    echo "Usage: sbatch ass1.sbatch <P>"
    exit 1
fi

# ---------------- Output ----------------
OUTFILE="results_P${P}.csv"
echo "M,P,Run,Time" > $OUTFILE

# ---------------- M values ----------------
for M in 262144 1048576; do
    for RUN in 1 2 3 4 5; do
        echo "P=$P M=$M Run=$RUN"

        mpirun -np $P ./ass1 $M $D1 $D2 $T $SEED \
            | awk -v m=$M -v p=$P -v r=$RUN '{print m","p","r","$3}' \
            >> $OUTFILE
    done
done

echo "Completed experiments for P=$P"
