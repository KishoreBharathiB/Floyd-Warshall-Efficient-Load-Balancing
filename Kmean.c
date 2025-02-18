#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>

#define N 200 // Number of nodes
#define K 10  // Number of clusters
#define MAX_BANDWIDTH 100
#define TRAFFIC_SIMULATIONS 5000 // Number of traffic simulations

typedef struct {
    double x, y;
} Node, Centroid;

Node nodes[N];
Centroid centroids[K];
int cluster_assignment[N];
int network[N][N];
int trafficLoad[N][N];

int matrix[200][200]; // Matrix data need to taken from the extracted matrix through dataExtraction.py python program.

// Function prototypes
void initializeNodes();
void initializeCentroids();
int assignClusters();
void updateCentroids();
void kMeansClustering();
double calculateDistance(Node n, Centroid c);
void simulateTraffic();
void saveLoadMatrixToCSV(int matrix[N][N], const char *filename);

void initializeNodes() {
    for (int i = 0; i < N; i++) {
        nodes[i].x = rand() % 100;
        nodes[i].y = rand() % 100;
    }
}

void initializeCentroids() {
    for (int i = 0; i < K; i++) {
        centroids[i] = (Centroid){rand() % 100, rand() % 100};
    }
}

void initializeNetwork() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            network[i][j] = matrix[i][j];
        }
    }
}

double calculateDistance(Node n, Centroid c) {
    return sqrt((n.x - c.x) * (n.x - c.x) + (n.y - c.y) * (n.y - c.y));
}

int assignClusters() {
    int changes = 0;
    for (int i = 0; i < N; i++) {
        double min_distance = DBL_MAX;
        int nearest_centroid = 0;
        for (int j = 0; j < K; j++) {
            double distance = calculateDistance(nodes[i], centroids[j]);
            if (distance < min_distance) {
                min_distance = distance;
                nearest_centroid = j;
            }
        }
        if (cluster_assignment[i] != nearest_centroid) {
            cluster_assignment[i] = nearest_centroid;
            changes++;
        }
    }
    return changes;
}

void updateCentroids() {
    int cluster_counts[K] = {0};
    Centroid new_centroids[K] = {0};
    for (int i = 0; i < N; i++) {
        int cluster_id = cluster_assignment[i];
        new_centroids[cluster_id].x += nodes[i].x;
        new_centroids[cluster_id].y += nodes[i].y;
        cluster_counts[cluster_id]++;
    }
    for (int j = 0; j < K; j++) {
        if (cluster_counts[j] > 0) {
            centroids[j].x = new_centroids[j].x / cluster_counts[j];
            centroids[j].y = new_centroids[j].y / cluster_counts[j];
        }
    }
}

void kMeansClustering() {
    initializeNodes();
    initializeCentroids();
    int changes;
    do {
        changes = assignClusters();
        updateCentroids();
    } while (changes > 0);
}

void saveLoadMatrixToCSV(int matrix[N][N], const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
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

void simulateTraffic() {
    int totalTraffic = 0;

    initializeNetwork();

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            trafficLoad[i][j] = 0;
        }
    }

    kMeansClustering();

    for (int trafficSimulation = 0; trafficSimulation < TRAFFIC_SIMULATIONS; trafficSimulation++) {
        int src = rand() % N;
        int dst = rand() % N;
        int src_cluster = cluster_assignment[src];
        int dst_cluster = cluster_assignment[dst];
        int packetSize = 1 + rand() % MAX_BANDWIDTH;

        if (src == dst) continue;

        // Printing traffic routing data
        printf("Traffic Simulation %d: Source = %d, Destination = %d, Packet Size = %d\n", 
                trafficSimulation + 1, src, dst, packetSize);

        if (src_cluster == dst_cluster) {
            if (network[src][dst] >= trafficLoad[src][dst] + packetSize) {
                trafficLoad[src][dst] += packetSize;
                totalTraffic += packetSize;
                printf("  Direct route used: Source -> Destination (Cluster %d)\n", src_cluster);
            }
        } else {
            int ch_src = src_cluster;
            int ch_dst = dst_cluster;

            // Routing through the clusters
            if (network[src][ch_src] >= trafficLoad[src][ch_src] + packetSize) {
                trafficLoad[src][ch_src] += packetSize;
                totalTraffic += packetSize;
                printf("  Route: Source -> Cluster %d\n", ch_src);
            }

            if (network[ch_src][ch_dst] >= trafficLoad[ch_src][ch_dst] + packetSize) {
                trafficLoad[ch_src][ch_dst] += packetSize;
                totalTraffic += packetSize;
                printf("  Route: Cluster %d -> Cluster %d\n", ch_src, ch_dst);
            }

            if (network[ch_dst][dst] >= trafficLoad[ch_dst][dst] + packetSize) {
                trafficLoad[ch_dst][dst] += packetSize;
                totalTraffic += packetSize;
                printf("  Route: Cluster %d -> Destination\n", ch_dst);
            }
        }
    }

    printf("Total traffic routed: %d\n", totalTraffic);

    // Print the traffic load matrix to the terminal
    printf("Traffic Load Matrix:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", trafficLoad[i][j]);
        }
        printf("\n");
    }

    saveLoadMatrixToCSV(trafficLoad, "traffic_load_matrix.csv");
}

int main() {
    clock_t start_time = clock();
    simulateTraffic();
    clock_t end_time = clock();
    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Execution time: %.2f seconds\n", execution_time);
    return 0;
}
