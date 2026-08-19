/*
 * Assignment 1: MPI Data Exchange (No Buffering / Blocking Implementation)
 *
 * Description:
 * This program performs data exchange using strict Blocking Communication (MPI_Send/MPI_Recv).
 * 
 * Logic:
 * 1. State: Maintains separate data arrays for D1 and D2 logic (No Addition update).
 * 2. Communication: Uses Block Parity scheduling (Phase A/B) to prevent deadlocks.
 * 3. Sequence: Send/Recv -> Compute -> Recv Back -> Update State -> Prepare Next Buffer.
 * 4. Output: Calculates global maximums for D1 and D2 streams separately.
 *
 * Compile: mpicc assignment1.c -o assignment1 -lm
 * Run:     mpirun -np <P> ./assignment1 <M> <D1> <D2> <T> <seed>
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

// Helper macros for tags to prevent message mix-ups
#define TAG_D1_FWD 10
#define TAG_D2_FWD 20
#define TAG_D1_BWD 30
#define TAG_D2_BWD 40
#define TAG_FINAL_COLLECT 100

#pragma prutor-mpi-args: -np 32 -ppn 16
#pragma prutor-mpi-sysargs: 1048576 2 4 10 1000

int main(int argc, char** argv) {
    int rank, size;
    int M, D1, D2, T, seed;
    
    // State and Buffer Arrays (Separate for D1 and D2)
    double *data_received_D1 = NULL; 
    double *data_received_D2 = NULL; 
    double *buf_send_D1 = NULL;
    double *buf_send_D2 = NULL;
    double *buf_recv_work_D1 = NULL;
    double *buf_recv_work_D2 = NULL;
    double *buf_res_send_D1 = NULL;
    double *buf_res_send_D2 = NULL;
    double *buf_res_recv_D1 = NULL;
    double *buf_res_recv_D2 = NULL;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 6) {
        if (rank == 0) fprintf(stderr, "Usage: %s <M> <D1> <D2> <T> <seed>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    M = atoi(argv[1]);
    D1 = atoi(argv[2]);
    D2 = atoi(argv[3]);
    T = atoi(argv[4]);
    seed = atoi(argv[5]);

    // Role Definition
    int is_sender_D1 = (rank + D1 < size);
    int is_sender_D2 = (rank + D2 < size);
    // Worker roles (receiving from left)
    int is_worker_D1 = (rank - D1 >= 0);
    int is_worker_D2 = (rank - D2 >= 0);

    // Memory Allocation
    data_received_D1 = (double*)malloc(M * sizeof(double));
    data_received_D2 = (double*)malloc(M * sizeof(double));
    buf_send_D1 = (double*)malloc(M * sizeof(double));
    buf_send_D2 = (double*)malloc(M * sizeof(double));
    buf_recv_work_D1 = (double*)malloc(M * sizeof(double));
    buf_recv_work_D2 = (double*)malloc(M * sizeof(double));
    buf_res_send_D1 = (double*)malloc(M * sizeof(double));
    buf_res_send_D2 = (double*)malloc(M * sizeof(double));
    buf_res_recv_D1 = (double*)malloc(M * sizeof(double));
    buf_res_recv_D2 = (double*)malloc(M * sizeof(double));

    if (!data_received_D1 || !data_received_D2 || !buf_send_D1 || !buf_send_D2 ||
        !buf_recv_work_D1 || !buf_recv_work_D2 || !buf_res_send_D1 || !buf_res_send_D2 ||
        !buf_res_recv_D1 || !buf_res_recv_D2) {
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Initialization
    srand(seed);
    for (int i = 0; i < M; i++) {
        double val = (double)rand() * (rank + 1) / 10000.0;
        data_received_D1[i] = val;
        data_received_D2[i] = val;
        
        // Prepare initial send buffers (Raw data for Iteration 0)
        // "Note that the initial data at the sender is the same for both sends"
        buf_send_D1[i] = val;
        buf_send_D2[i] = val;
    }

    double start_time = MPI_Wtime();

    for (int iter = 0; iter < T; iter++) {
        MPI_Status status;
        int block_id;

        // ==========================================================
        // Step 1: Forward Communication (Block Parity Scheduling)
        // Send currently prepared buffers to workers
        // ==========================================================
        
        // --- Handle D1 Forward ---
        block_id = rank / D1;
        // Phase A: Even blocks send, Odd blocks receive
        if (block_id % 2 == 0) {
            if (is_sender_D1) MPI_Send(buf_send_D1, M, MPI_DOUBLE, rank + D1, TAG_D1_FWD, MPI_COMM_WORLD);
        } else {
            if (is_worker_D1) MPI_Recv(buf_recv_work_D1, M, MPI_DOUBLE, rank - D1, TAG_D1_FWD, MPI_COMM_WORLD, &status);
        }
        // Phase B: Odd blocks send, Even blocks receive
        if (block_id % 2 == 1) {
            if (is_sender_D1) MPI_Send(buf_send_D1, M, MPI_DOUBLE, rank + D1, TAG_D1_FWD, MPI_COMM_WORLD);
        } else {
            if (is_worker_D1) MPI_Recv(buf_recv_work_D1, M, MPI_DOUBLE, rank - D1, TAG_D1_FWD, MPI_COMM_WORLD, &status);
        }

        // --- Handle D2 Forward ---
        block_id = rank / D2;
        // Phase A: Even blocks send, Odd blocks receive
        if (block_id % 2 == 0) {
            if (is_sender_D2) MPI_Send(buf_send_D2, M, MPI_DOUBLE, rank + D2, TAG_D2_FWD, MPI_COMM_WORLD);
        } else {
            if (is_worker_D2) MPI_Recv(buf_recv_work_D2, M, MPI_DOUBLE, rank - D2, TAG_D2_FWD, MPI_COMM_WORLD, &status);
        }
        // Phase B: Odd blocks send, Even blocks receive
        if (block_id % 2 == 1) {
            if (is_sender_D2) MPI_Send(buf_send_D2, M, MPI_DOUBLE, rank + D2, TAG_D2_FWD, MPI_COMM_WORLD);
        } else {
            if (is_worker_D2) MPI_Recv(buf_recv_work_D2, M, MPI_DOUBLE, rank - D2, TAG_D2_FWD, MPI_COMM_WORLD, &status);
        }

        // ==========================================================
        // Step 2: Computation (Worker Side)
        // ==========================================================
        if (is_worker_D1) {
            for (int i = 0; i < M; i++) 
                buf_res_send_D1[i] = buf_recv_work_D1[i] * buf_recv_work_D1[i];
        }
        if (is_worker_D2) {
            for (int i = 0; i < M; i++) {
                double val = buf_recv_work_D2[i];
                buf_res_send_D2[i] = (val > 0) ? log(val) : 0.0;
            }
        }

        // ==========================================================
        // Step 3: Backward Communication (Results)
        // Reverse Roles: Worker (left) sends back to Sender (right)
        // ==========================================================

        // --- Handle D1 Backward ---
        block_id = rank / D1;
        // Phase A: Odd (Workers) Send Back, Even (Senders) Recv Back
        if (block_id % 2 == 1) {
            if (is_worker_D1) MPI_Send(buf_res_send_D1, M, MPI_DOUBLE, rank - D1, TAG_D1_BWD, MPI_COMM_WORLD);
        } else {
            if (is_sender_D1) MPI_Recv(buf_res_recv_D1, M, MPI_DOUBLE, rank + D1, TAG_D1_BWD, MPI_COMM_WORLD, &status);
        }
        // Phase B: Even (Workers) Send Back, Odd (Senders) Recv Back
        if (block_id % 2 == 0) {
            if (is_worker_D1) MPI_Send(buf_res_send_D1, M, MPI_DOUBLE, rank - D1, TAG_D1_BWD, MPI_COMM_WORLD);
        } else {
            if (is_sender_D1) MPI_Recv(buf_res_recv_D1, M, MPI_DOUBLE, rank + D1, TAG_D1_BWD, MPI_COMM_WORLD, &status);
        }

        // --- Handle D2 Backward ---
        block_id = rank / D2;
        // Phase A: Odd (Workers) Send Back, Even (Senders) Recv Back
        if (block_id % 2 == 1) {
            if (is_worker_D2) MPI_Send(buf_res_send_D2, M, MPI_DOUBLE, rank - D2, TAG_D2_BWD, MPI_COMM_WORLD);
        } else {
            if (is_sender_D2) MPI_Recv(buf_res_recv_D2, M, MPI_DOUBLE, rank + D2, TAG_D2_BWD, MPI_COMM_WORLD, &status);
        }
        // Phase B: Even (Workers) Send Back, Odd (Senders) Recv Back
        if (block_id % 2 == 0) {
            if (is_worker_D2) MPI_Send(buf_res_send_D2, M, MPI_DOUBLE, rank - D2, TAG_D2_BWD, MPI_COMM_WORLD);
        } else {
            if (is_sender_D2) MPI_Recv(buf_res_recv_D2, M, MPI_DOUBLE, rank + D2, TAG_D2_BWD, MPI_COMM_WORLD, &status);
        }

        // ==========================================================
        // Step 4: Update State (No Addition)
        // ==========================================================
        if (is_sender_D1) {
            for (int i = 0; i < M; i++) data_received_D1[i] = buf_res_recv_D1[i];
        }
        if (is_sender_D2) {
            for (int i = 0; i < M; i++) data_received_D2[i] = buf_res_recv_D2[i];
        }

        // ==========================================================
        // Step 5: Prepare Send Buffers for NEXT iteration
        // "data to be sent ... in the next iteration is updated as..."
        // ==========================================================
        if (is_sender_D1) {
            for (int i = 0; i < M; i++) 
                buf_send_D1[i] = (double)((unsigned long long)data_received_D1[i] % 100000);
        }
        if (is_sender_D2) {
            for (int i = 0; i < M; i++) 
                buf_send_D2[i] = data_received_D2[i] * 100000.0;
        }

    } // End of Loop

    // ==========================================================
    // Final Data Collection
    // ==========================================================
    
    // Find local maximums for the final state
    double local_max_D1 = -1.0e300; 
    double local_max_D2 = -1.0e300;

    // We only consider data valid if this rank is a sender for that specific distance
    if (is_sender_D1) {
        for (int i = 0; i < M; i++) if (buf_send_D1[i] > local_max_D1) local_max_D1 = buf_send_D1[i];
    }
    if (is_sender_D2) {
        for (int i = 0; i < M; i++) if (buf_send_D2[i] > local_max_D2) local_max_D2 = buf_send_D2[i];
    }

    double global_max_D1 = local_max_D1;
    double global_max_D2 = local_max_D2;

    // Collect all maximums at Rank 0
    if (rank == 0) {
        // Rank 0 acts as a server collecting data from all other ranks
        for (int src = 1; src < size; src++) {
            double incoming[2];
            MPI_Status status;
            MPI_Recv(incoming, 2, MPI_DOUBLE, src, TAG_FINAL_COLLECT, MPI_COMM_WORLD, &status);
            
            // Update Global Maximums
            if (incoming[0] > global_max_D1) global_max_D1 = incoming[0];
            if (incoming[1] > global_max_D2) global_max_D2 = incoming[1];
        }
    } else {
        // All other ranks send their local maximums to Rank 0
        double outgoing[2];
        outgoing[0] = local_max_D1;
        outgoing[1] = local_max_D2;
        MPI_Send(outgoing, 2, MPI_DOUBLE, 0, TAG_FINAL_COLLECT, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();
    double total_time = end_time - start_time;
    double max_time = 0.0;
    
    // Reduce time using MPI_Reduce (allowed for time)
    MPI_Reduce(&total_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("%lf %lf %lf\n", global_max_D1, global_max_D2, max_time);
    }

    // Cleanup
    free(data_received_D1); free(data_received_D2); free(buf_send_D1); free(buf_send_D2);
    free(buf_recv_work_D1); free(buf_recv_work_D2);
    free(buf_res_send_D1); free(buf_res_send_D2);
    free(buf_res_recv_D1); free(buf_res_recv_D2);

    MPI_Finalize();
    return 0;
}
