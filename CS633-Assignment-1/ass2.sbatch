#!/bin/bash
#SBATCH --job-name=mpi_assign1
#SBATCH -N 2
#SBATCH --ntasks-per-node=16
#SBATCH --partition=cpu
#SBATCH --time=00:05:00
#SBATCH --output=assign1_3_%j.out
#SBATCH --error=assign1_3_%j.err

module load compiler/oneapi-2024/mpi

# ---------------- Compile ----------------
# We are commenting this OUT with '#' because you compile manually.
# If you ever want to compile inside the job, remove the '#' and load the compiler module.
# mpicc assignment1.c -o assignment1 -lm
# if [ $? -ne 0 ]; then
#     echo "Compilation failed"
#     exit 1
# fi

# ---------------- Fixed parameters ----------------
D1=2
D2=4
T=10
SEED=1000

# ---------------- Process count ----------------
P=$1
if [ -z "$P" ]; then
    echo "Error: Usage sbatch ass2.sbatch <P>"
    exit 1
fi

# ---------------- Output ----------------
OUTFILE="results_P${P}.csv"
# Header for the CSV file
echo "M,P,Run,Time" > $OUTFILE

echo "Starting experiments for P=$P..."

# ---------------- M values ----------------
for M in 262144 1048576; do
    for RUN in 1 2 3 4 5; do
        echo "Running: P=$P M=$M Run=$RUN"

        # 1. Run the program and capture output
        # IMPORTANT: Make sure your compiled file is named 'assignment1'
        # If your compiled file is 'ass1_3', change './assignment1' to './ass1_3' below:
        RESULT=$(mpirun -np $P ./assfinal $M $D1 $D2 $T $SEED)
        
        # 2. Print result to SLURM output file (so you can verify MaxD1/MaxD2)
        echo "Output: $RESULT"

        # 3. Extract just the time (3rd column) for the CSV
        TIME_VAL=$(echo "$RESULT" | awk '{print $3}')

        # 4. Save to CSV
        echo "$M,$P,$RUN,$TIME_VAL" >> $OUTFILE
    done
done

echo "Completed experiments for P=$P"
