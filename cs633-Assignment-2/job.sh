#!/bin/bash

#SBATCH --job-name=group_33_1
#SBATCH -N 2
#SBATCH --ntasks-per-node=16
#SBATCH --output=group_33_1_%j.out
#SBATCH --error=group_33_1_%j.err
#SBATCH --partition=cpu
#SBATCH --time=00:10:00

module load compiler/oneapi-2024/mpi

for execution in {1..5}; do
  for P in 8 16 32; do
    for M in $((512*512)) $((1024*1024)); do
      for D1 in 2; do
        for D2 in 4; do
          for T in 10; do
            for seed in 1000; do

              echo "Run $execution | P=$P M=$M D1=$D1 D2=$D2 T=$T seed=$seed"

              mpirun -np "$P" ./src \
                     "$M" "$D1" "$D2" "$T" "$seed"

              printf '\n'
            done
          done
        done
      done
    done
  done
done
