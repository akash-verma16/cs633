#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank,size;

    // MPI Initialization 
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Bad Usage Output
    if (argc != 6) {
        if (rank == 0)
            printf("You SUCK !! . Usage: %s M D1 D2 T seed\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    // Fetch arguments
    int M = atoi(argv[1]);
    int D1 = atoi(argv[2]);
    int D2 = atoi(argv[3]);
    int T = atoi(argv[4]);
    int seed = atoi(argv[5]);

    // Memory Declaration
    double *data_received = (double*)malloc(M *sizeof(double));
    double *buffer_D1 = (double*)malloc(M*sizeof(double));
    double *buffer_D2 = (double*)malloc(M* sizeof(double));

    double *recieved_D1 = (double*)malloc(M*sizeof(double));
    double *recveived_D2 = (double*)malloc(M *sizeof(double));

    // Start the process
    srand(seed + rank);
    for (int i=0; i<M;i++) {
        data_received[i] = (double)rand() * (rank+1)/10000.0;
        buffer_D1[i] =data_received[i];
        buffer_D2[i]= data_received[i];
    }

    // Calculate whats needed to be put in Mpi send and recieve
    int destination_D1 = rank+D1;
    int destination_D2 = rank + D2;
    int source_D1 = rank -D1;
    int source_D2 = rank- D2;

    // Validity to send of not
    int send_D1_valid = (destination_D1 <size);
    int send_D2_valid = (destination_D2< size);
    int rec_D1_valid = (source_D1>= 0);
    int rec_D2_valid = (source_D2 >=0);

    // Start time count
    MPI_Barrier(MPI_COMM_WORLD);
    double starttime = MPI_Wtime();

    // Main SHIT. Might be dirty code 
    for (int t=0;t<T;t++) {

        // FWD
        if (send_D1_valid)
            MPI_Send(buffer_D1, M,MPI_DOUBLE, destination_D1,100, MPI_COMM_WORLD);

        if (send_D2_valid)
            MPI_Send(buffer_D2,M, MPI_DOUBLE, destination_D2, 200,MPI_COMM_WORLD);

        if (rec_D1_valid)
            MPI_Recv(recieved_D1,M, MPI_DOUBLE,source_D1, 100,MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (rec_D2_valid)
            MPI_Recv(recveived_D2,M, MPI_DOUBLE, source_D2,200, MPI_COMM_WORLD,MPI_STATUS_IGNORE);

        // REC COMM
        if (rec_D1_valid) {
            for (int i=0;i <M;i++)
                recieved_D1[i] = recieved_D1[i] * recieved_D1[i];
        }

        if (rec_D2_valid) {
            for (int i = 0; i<M; i++)
                recveived_D2[i] = log(fmax(recveived_D2[i], 1e-12));
        }

        // BWD
        if (rec_D1_valid)
            MPI_Send(recieved_D1, M,MPI_DOUBLE, source_D1, 300, MPI_COMM_WORLD);

        if (rec_D2_valid)
            MPI_Send(recveived_D2,M, MPI_DOUBLE,source_D2, 400, MPI_COMM_WORLD);

        if (send_D1_valid)
            MPI_Recv(buffer_D1, M, MPI_DOUBLE,destination_D1, 300, MPI_COMM_WORLD,MPI_STATUS_IGNORE);

        if (send_D2_valid)
            MPI_Recv(buffer_D2, M, MPI_DOUBLE,destination_D2, 400,MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // SENDER UPDATE
        if (send_D1_valid && send_D2_valid) {
            for (int i =0;i<M; i++)
                data_received[i] = buffer_D1[i] + buffer_D2[i];
        } else if (send_D1_valid) {
            for (int i= 0; i <M;i++)
                data_received[i] = buffer_D1[i];
        }

        if (send_D1_valid) {
            for (int i = 0;i <M;i++)
                buffer_D1[i] = (double)((unsigned long long)data_received[i] % 100000ULL);
        }

        if (send_D2_valid) {
            for (int i = 0; i < M;i++)
                buffer_D2[i] = data_received[i] * 100000.0;
        }
    }

    // Process complete. finding time
    double endtime = MPI_Wtime();
    double local_time = endtime - starttime;

    // FINAL Computation
    double local_max_D1 = -INFINITY;
    double local_max_D2 = -INFINITY;

    if (send_D1_valid) {
        local_max_D1 = buffer_D1[0];
        for (int i = 1; i < M; i++)
            if (buffer_D1[i] > local_max_D1) local_max_D1 = buffer_D1[i];
    }

    if (send_D2_valid) {
        local_max_D2 = buffer_D2[0];
        for (int i = 1; i < M; i++)
            if (buffer_D2[i] > local_max_D2) local_max_D2 = buffer_D2[i];
    }

    double global_max_D1, global_max_D2, global_time;
    
    MPI_Reduce(&local_max_D1,&global_max_D1, 1, MPI_DOUBLE,MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_max_D2, &global_max_D2, 1, MPI_DOUBLE, MPI_MAX, 0,MPI_COMM_WORLD);
    MPI_Reduce(&local_time,&global_time,1, MPI_DOUBLE,MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0)
        printf("%.6f %.6f %.6f\n", global_max_D1, global_max_D2, global_time);

    // Free the malloc memory
    free(data_received);
    free(buffer_D1);
    free(buffer_D2);
    free(recieved_D1);
    free(recveived_D2);

    // Ho gya !!!
    MPI_Finalize();
    return 0;
}
