#!/bin/bash

# Compile the code
mpicc assignment1.c -o assignment1 -lm

# Create output file
output_file="experiment_results.csv"
echo "M,P,Time" > $output_file

# Fixed parameters
D1=2
D2=4
T=10
SEED=1000

# Experiment Loops 
# M values: 262144, 1048576
for M in 262144 1048576; do
    # P values: 8, 16, 32
    for P in 8 16 32; do
        echo "Running configuration: M=$M, P=$P"
        
        # Repeat 5 times 
        for (( i=1; i<=5; i++ )); do
            # Run MPI program
            # Note: Assuming hostfile exists as per command 
            # If running locally without hostfile, remove "-f hostfile"
            
            output=$(mpirun --oversubscribe -np $P ./assignment1 $M $D1 $D2 $T $SEED)
            # output=$(mpirun -np $P ./assignment1 $M $D1 $D2 $T $SEED) # Use this line in supercomputer environment


            # Extract time (3rd value) from output
            # Output format: <maxD1> <maxD2> <time>
            time_val=$(echo $output | awk '{print $3}')
            
            # Save to CSV
            echo "$M,$P,$time_val" >> $output_file
        done
    done
done

echo "Experiments completed. Results saved to $output_file"