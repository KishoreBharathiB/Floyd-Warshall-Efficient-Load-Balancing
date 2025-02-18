#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <omp.h>

#define N 150 // Number of nodes in the network
#define INF 99999 // Infinite distance for unreachable nodes
#define MAX_BANDWIDTH 100 // Maximum bandwidth for any link
#define TRAFFIC_SIMULATIONS 5000 // Number of traffic simulations

// Define a network graph (N x N matrix for simplicity)
float network[501][501]; // Take the input from the extracted matrix

// Bandwidth matrix for each link
int bandwidth[N][N];

// Traffic load on each link
int trafficLoad[N][N];

// Function to initialize the trafficload and bandwidth matrixs
void initializeNetwork() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            bandwidth[i][j] = MAX_BANDWIDTH; // Initialize bandwidth for all links
            trafficLoad[i][j] = 0; // Initialize traffic load to 0
        }
    }
}

// Function to save matrix to a CSV file
void saveMatrixToCSV(int matrix[N][N], const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Unable to open file for writing.");
        return;
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(file, "%d", matrix[i][j]);
            if (j < N - 1) {
                fprintf(file, ",");
            }
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("Traffic load matrix saved to %s\n", filename);
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
    int i, j, k;

    #pragma omp parallel for private(i, j)
    for (k = 0; k < N; k++) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
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
void simulateTraffic(int dist[N][N]) {
    int totalTraffic = 0;
    char *messageBuffer[TRAFFIC_SIMULATIONS]; // Buffer for messages
    int messageIndex = 0;

    #pragma omp parallel num_threads(2)
    {
        // Thread-local variables
        int localTraffic = 0;
        char *localMessages[TRAFFIC_SIMULATIONS]; // Local messages for each thread
        int localMessageCount = 0;

        #pragma omp for
        for (int trafficSimulation = 0; trafficSimulation < TRAFFIC_SIMULATIONS; trafficSimulation++) {
            int src = rand() % N;
            int dst = rand() % N;

            if (src != dst && dist[src][dst] != INF) {
                int packetSize = 1 + rand() % MAX_BANDWIDTH;

                if (bandwidth[src][dst] >= trafficLoad[src][dst] + packetSize) {
                    #pragma omp atomic
                    trafficLoad[src][dst] += packetSize;
                    localTraffic += packetSize;
                    
                    #pragma omp critical
                    {
                        localMessages[localMessageCount] = (char*)malloc(256 * sizeof(char));
                        sprintf(localMessages[localMessageCount], "Traffic routed on primary path from %d -> %d. Current load: %d\n", src + 1, dst + 1, trafficLoad[src][dst]);
                        localMessageCount++;
                    }
                } else {
                    for (int i = 0; i < N; i++) {
                        if (i != src && dist[src][i] != INF && dist[i][dst] != INF) {
                            if (bandwidth[src][i] >= trafficLoad[src][i] + packetSize) {
                                #pragma omp atomic
                                trafficLoad[src][i] += packetSize;
                                localTraffic += packetSize;
                                
                                #pragma omp critical
                                {
                                    localMessages[localMessageCount] = (char*)malloc(256 * sizeof(char));
                                    sprintf(localMessages[localMessageCount], "Traffic rerouted on path from %d -> %d -> %d. Current load: %d\n", src + 1, i + 1, dst + 1, trafficLoad[src][i]);
                                    localMessageCount++;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }

        #pragma omp critical
        {
            for (int j = 0; j < localMessageCount; j++) {
                messageBuffer[messageIndex] = localMessages[j];
                messageIndex++;
            }
        }
    }

    for (int i = 0; i < messageIndex; i++) {
        printf("%s", messageBuffer[i]);
        free(messageBuffer[i]);
    }

    printMatrix(trafficLoad, "Final Traffic Load Matrix");

    saveMatrixToCSV(trafficLoad, "traffic_load.csv"); // Save traffic load matrix to CSV
}

int main() {
    int dist[N][N];

    initializeNetwork();

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            dist[i][j] = network[i][j];
        }
    }

    double total_start_time = omp_get_wtime();

    double start_time = omp_get_wtime();
    floydWarshall(dist);
    double end_time = omp_get_wtime();
    printf("\nTime taken for parallel Floyd-Warshall: %.4f seconds\n", end_time - start_time);

    printMatrix(dist, "Shortest Distance Matrix");

    simulateTraffic(dist);

    double total_end_time = omp_get_wtime();
    printf("\nTotal execution time of the program: %.4f seconds\n", total_end_time - total_start_time);

    return 0;
}
