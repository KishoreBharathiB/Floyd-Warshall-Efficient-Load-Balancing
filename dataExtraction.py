import numpy as np

def generate_network_matrix(filename="data1.txt", N=100, min_distance=5, max_distance=30):
    # Initialize the matrix with 'inf' to represent no direct path between nodes
    network_matrix = np.full((N, N), float('inf'))

    # Read the file and populate the matrix based on start and end nodes
    with open(filename, "r") as file:
        for line in file:
            parts = line.split()
            if len(parts) < 4:
                continue  # Skip any malformed line
            start_node, end_node, distance = int(parts[1]), int(parts[2]), float(parts[3])
            if start_node < N and end_node < N:
                network_matrix[start_node][end_node] = distance
                network_matrix[end_node][start_node] = distance  # Assuming an undirected graph

    # Set diagonal entries to 0 (distance from each node to itself)
    np.fill_diagonal(network_matrix, 0)

    network_matrix[np.isinf(network_matrix)] = np.random.randint(min_distance, max_distance - 15, size=np.sum(np.isinf(network_matrix)))

    # Save the matrix in C syntax
    with open(f"network_matrix_{N}x{N}_c_syntax.txt", "w") as f:
        f.write(f"float network_matrix[{N}][{N}] = {{\n")
        for row in network_matrix:
            f.write("{" + ", ".join(map(str, row)) + "},\n")
        f.write("};")

    print(f"Network matrix of {N}x{N} saved in C syntax!")

# Define the number of matrix entires to extract N X N
generate_network_matrix("data.txt", "Define the integer value N here")
