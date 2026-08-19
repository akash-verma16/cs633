#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

/* * Assignment 1 Solution
 * Validates against:
 * - Data generation formula 
 * - Communication rules for D1/D2 
 * - Receiver computations (square and log) 
 * - Sender updates (add, then mod/mul) 
 */

int main(int argc, char *argv[]) {
    int rank, size;
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //  Input: M, D1, D2, T, seed
    if (argc != 6) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s M D1 D2 T seed\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    int M = atoi(argv[1]);
    int D1 = atoi(argv[2]);
    int D2 = atoi(argv[3]);
    int T = atoi(argv[4]);
    int seed = atoi(argv[5]);

    // Memory Allocation
    double *data_received = (double *)malloc(M * sizeof(double));
    double *buf_send_D1 = (double *)malloc(M * sizeof(double));
    double *buf_send_D2 = (double *)malloc(M * sizeof(double));
    
    // Buffers for receiving from left (acting as Receiver)
    double *recv_from_left_D1 = (double *)malloc(M * sizeof(double));
    double *recv_from_left_D2 = (double *)malloc(M * sizeof(double));

    // Buffers for receiving results back from right (acting as Sender)
    double *result_from_right_D1 = (double *)malloc(M * sizeof(double));
    double *result_from_right_D2 = (double *)malloc(M * sizeof(double));

    // Random Generation
    srand(seed);
    for (int i = 0; i < M; i++) {
        //  data_received formula
        data_received[i] = (double)rand() * (rank + 1) / 10000.0;
        
        // Initial data to send is the same as generated
        buf_send_D1[i] = data_received[i];
        buf_send_D2[i] = data_received[i];
    }

    // Determine neighbors
    int dest_D1 = rank + D1;
    int dest_D2 = rank + D2;
    int src_D1 = rank - D1; // Process that is D1 away to my left
    int src_D2 = rank - D2; // Process that is D2 away to my left

    // Validate destinations
    int send_D1_valid = (dest_D1 < size);
    int send_D2_valid = (dest_D2 < size);
    int recv_D1_valid = (src_D1 >= 0);
    int recv_D2_valid = (src_D2 >= 0);

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    for (int t = 0; t < T; t++) {
        MPI_Request reqs[8];
        int req_count = 0;

        // --- STEP 1: Send Data to Neighbors (Forward Pass) ---
        // I am Sender: Send to my right
        if (send_D1_valid) {
            MPI_Isend(buf_send_D1, M, MPI_DOUBLE, dest_D1, 100, MPI_COMM_WORLD, &reqs[req_count++]);
        }
        if (send_D2_valid) {
            MPI_Isend(buf_send_D2, M, MPI_DOUBLE, dest_D2, 200, MPI_COMM_WORLD, &reqs[req_count++]);
        }

        // I am Receiver: Receive from my left
        if (recv_D1_valid) {
            MPI_Irecv(recv_from_left_D1, M, MPI_DOUBLE, src_D1, 100, MPI_COMM_WORLD, &reqs[req_count++]);
        }
        if (recv_D2_valid) {
            MPI_Irecv(recv_from_left_D2, M, MPI_DOUBLE, src_D2, 200, MPI_COMM_WORLD, &reqs[req_count++]);
        }

        MPI_Waitall(req_count, reqs, MPI_STATUSES_IGNORE);

        // --- STEP 2: Computation (Receiver Side) ---
        // 
        if (recv_D1_valid) {
            for (int i = 0; i < M; i++) {
                // Square the data
                recv_from_left_D1[i] = recv_from_left_D1[i] * recv_from_left_D1[i];
            }
        }
        if (recv_D2_valid) {
            for (int i = 0; i < M; i++) {
                // Log the data
                // Safety check: log of negative/zero is undefined. 
                // Assuming data stays positive based on initialization, but good practice to handle.
                if (recv_from_left_D2[i] > 0)
                    recv_from_left_D2[i] = log(recv_from_left_D2[i]);
                else 
                    recv_from_left_D2[i] = 0;
            }
        }

        // --- STEP 3: Send Results Back (Backward Pass) ---
        req_count = 0;

        // I am Receiver: Send back to left
        if (recv_D1_valid) {
            MPI_Isend(recv_from_left_D1, M, MPI_DOUBLE, src_D1, 300, MPI_COMM_WORLD, &reqs[req_count++]);
        }
        if (recv_D2_valid) {
            MPI_Isend(recv_from_left_D2, M, MPI_DOUBLE, src_D2, 400, MPI_COMM_WORLD, &reqs[req_count++]);
        }

        // I am Sender: Receive results from right
        if (send_D1_valid) {
            MPI_Irecv(result_from_right_D1, M, MPI_DOUBLE, dest_D1, 300, MPI_COMM_WORLD, &reqs[req_count++]);
        }
        if (send_D2_valid) {
            MPI_Irecv(result_from_right_D2, M, MPI_DOUBLE, dest_D2, 400, MPI_COMM_WORLD, &reqs[req_count++]);
        }

        MPI_Waitall(req_count, reqs, MPI_STATUSES_IGNORE);

        // --- STEP 4: Update Data (Sender Side) ---
        // "Sender adds data_at_D1 and data_at_D2 and updates data_received"
        // Note: Only update if we actually sent/received data.
        if (send_D1_valid && send_D2_valid) {
            for (int i = 0; i < M; i++) {
                data_received[i] = result_from_right_D1[i] + result_from_right_D2[i];
            }
        } else if (send_D1_valid) {
            //  If rank + D2 > P-1, only sends to D1.
            // Logic implies we only get D1 back.
            for (int i = 0; i < M; i++) {
                data_received[i] = result_from_right_D1[i]; 
            }
        } 
        // If no valid senders (invalid sender), data_received remains stagnant or unused.

        // Prepare buffers for NEXT iteration 
        if (send_D1_valid) {
            for (int i = 0; i < M; i++) {
                // (unsigned long long) cast then modulo 100000
                buf_send_D1[i] = (double)((unsigned long long)data_received[i] % 100000);
            }
        }
        if (send_D2_valid) {
            for (int i = 0; i < M; i++) {
                // multiply by 100000
                buf_send_D2[i] = data_received[i] * 100000.0;
            }
        }
    }

    double end_time = MPI_Wtime();
    double total_time = end_time - start_time;

    // --- Final Max Calculation ---
    // Max of final data_received_D1 and data_received_D2
    double local_max_D1 = -1.0;
    double local_max_D2 = -1.0;

    if (send_D1_valid) {
        local_max_D1 = result_from_right_D1[0];
        for(int i=1; i<M; i++) {
            if(result_from_right_D1[i] > local_max_D1) local_max_D1 = result_from_right_D1[i];
        }
    }

    if (send_D2_valid) {
        local_max_D2 = result_from_right_D2[0];
        for(int i=1; i<M; i++) {
            if(result_from_right_D2[i] > local_max_D2) local_max_D2 = result_from_right_D2[i];
        }
    }

    // Reduce to Rank 0
    double global_max_D1, global_max_D2, global_time;
    MPI_Reduce(&local_max_D1, &global_max_D1, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_max_D2, &global_max_D2, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    // Use MAX for time to get the time of the slowest process (standard for parallel metrics)
    MPI_Reduce(&total_time, &global_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // Output single line
    if (rank == 0) {
        printf("%.6f %.6f %.6f\n", global_max_D1, global_max_D2, global_time);
    }

    // Cleanup
    free(data_received);
    free(buf_send_D1);
    free(buf_send_D2);
    free(recv_from_left_D1);
    free(recv_from_left_D2);
    free(result_from_right_D1);
    free(result_from_right_D2);

    MPI_Finalize();
    return 0;
}