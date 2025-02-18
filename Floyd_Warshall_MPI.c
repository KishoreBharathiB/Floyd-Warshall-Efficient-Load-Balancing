#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <mpi.h>

#define N 200 // Number of nodes in the network
#define INF 99999 // Infinite distance for unreachable nodes
#define MAX_BANDWIDTH 100 // Maximum bandwidth for any link
#define TRAFFIC_SIMULATIONS 5000 // Number of traffic simulations

// Define a network graph (N x N matrix for simplicity)
int network[N][N]; // Here the data obtained form the extraction funciton need to be used

// Bandwidth matrix for each link
int bandwidth[N][N];

// Traffic load on each link
int trafficLoad[N][N];

// Function to initialize the network and bandwidth
void initializeNetwork() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == j) {
                network[i][j] = 0; // Distance to itself is zero
            } else {
                // Random distances between 1 and 20
                network[i][j] = 1 + rand() % 20; 
            }

            bandwidth[i][j] = MAX_BANDWIDTH; // Initialize bandwidth for all links
            trafficLoad[i][j] = 0; // Initialize traffic load to 0
        }
    }
}

// Function to print the matrix
void printMatrix(int matrix[N][N], const char *label) {
    printf("\n%s:\n", label);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (matrix[i][j] == INF) {
                printf(" INF ");
            } else {
                printf("%4d ", matrix[i][j]);
            }
        }
        printf("\n");
    }
}

// Floyd-Warshall Algorithm for finding shortest paths
void floydWarshall(int dist[N][N]) {
    for (int k = 0; k < N; k++) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (dist[i][k] != INF && dist[k][j] != INF) {
                    int newDist = dist[i][k] + dist[k][j];
                    if (dist[i][j] > newDist) {
                        dist[i][j] = newDist;
                    }
                }
            }
        }
    }
}

// Simulate realistic traffic demand and load balancing
void simulateTraffic(int dist[N][N], int rank, int size) {
    int totalTraffic = 0;

    for (int trafficSimulation = rank; trafficSimulation < TRAFFIC_SIMULATIONS; trafficSimulation += size) {
        // Randomly select a source and destination node
        int src = rand() % N;
        int dst = rand() % N;

        // Ensure src and dst are not the same
        if (src != dst && dist[src][dst] != INF) {
            // Random packet size between 1 and MAX_BANDWIDTH to ensure diverse traffic
            int packetSize = 1 + rand() % MAX_BANDWIDTH;

            // Check if the link can handle the traffic
            if (bandwidth[src][dst] >= trafficLoad[src][dst] + packetSize) {
                trafficLoad[src][dst] += packetSize;
                totalTraffic += packetSize; // Count total traffic for statistics
            } else {
                // Try to reroute traffic on alternative paths
                for (int i = 0; i < N; i++) {
                    if (i != src && dist[src][i] != INF && dist[i][dst] != INF) {
                        // Check if this alternate path can handle the packet size
                        if (bandwidth[src][i] >= trafficLoad[src][i] + packetSize) {
                            trafficLoad[src][i] += packetSize;
                            totalTraffic += packetSize; // Count total traffic for statistics
                            break; // Exit after rerouting once
                        }
                    }
                }
            }
        }
    }

    // Gather total traffic from all processes
    int globalTotalTraffic;
    MPI_Reduce(&totalTraffic, &globalTotalTraffic, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // Print total traffic routed from process 0
    if (rank == 0) {
        printf("\nTotal Traffic Routed: %d\n", globalTotalTraffic);
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int dist[N][N];
    
    // Initialize the network and bandwidth
    initializeNetwork();

    // Broadcast the network matrix to all processes
    MPI_Bcast(network, N*N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(bandwidth, N*N, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Copy the network matrix to the distance matrix
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            dist[i][j] = network[i][j];
        }
    }

    // Measure the total execution time of the program
    double total_start_time = MPI_Wtime();

    // Measure the time taken for the Floyd-Warshall algorithm
    double start_time = MPI_Wtime();

    // Apply Floyd-Warshall to find shortest paths
    floydWarshall(dist);

    double end_time = MPI_Wtime();
    if (rank == 0) {
        printf("\nTime taken for Floyd-Warshall: %.4f seconds\n", end_time - start_time);
        printMatrix(dist, "Shortest Distance Matrix");
    }

    // Simulate load balancing and traffic engineering
    simulateTraffic(dist, rank, size);

    // Measure the total execution time after simulation
    double total_end_time = MPI_Wtime();
    if (rank == 0) {
        printf("\nTotal execution time of the program: %.4f seconds\n", total_end_time - total_start_time);
    }

    MPI_Finalize();
    return 0;
}
