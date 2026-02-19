#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NODES 20
#define NAME_LEN 10
#define INF 999999 // Represents Infinity for unconnected nodes

typedef struct {
    char names[MAX_NODES][NAME_LEN];
    int adj_matrix[MAX_NODES][MAX_NODES];
    int num_nodes;
} NetworkGraph;

// --- Graph Initialization ---
void init_network(NetworkGraph* g) {
    g->num_nodes = 0;
    for (int i = 0; i < MAX_NODES; i++) {
        for (int j = 0; j < MAX_NODES; j++) {
            g->adj_matrix[i][j] = (i == j) ? 0 : INF;
        }
    }
}

// --- Helper: Get Index from Name ---
int get_node_index(NetworkGraph* g, const char* name) {
    for (int i = 0; i < g->num_nodes; i++) {
        if (strcmp(g->names[i], name) == 0) return i;
    }
    return -1;
}

// --- Add Node and Edge ---
int add_node(NetworkGraph* g, const char* name) {
    int idx = get_node_index(g, name);
    if (idx != -1) return idx;

    if (g->num_nodes >= MAX_NODES) {
        printf("Error: Max network nodes reached.\n");
        return -1;
    }
    strcpy(g->names[g->num_nodes], name);
    return g->num_nodes++;
}

void add_link(NetworkGraph* g, const char* u_name, const char* v_name, int latency) {
    int u = add_node(g, u_name);
    int v = add_node(g, v_name);

    if (u != -1 && v != -1) {
        // Bidirectional link
        g->adj_matrix[u][v] = latency;
        g->adj_matrix[v][u] = latency;
    }
}

// --- Dijkstra's Algorithm ---
void find_shortest_path(NetworkGraph* g, const char* start_name, const char* target_name) {
    int start = get_node_index(g, start_name);
    int target = get_node_index(g, target_name);

    if (start == -1 || target == -1) {
        printf("Error: Invalid starting or destination server name.\n");
        return;
    }

    int dist[MAX_NODES];     // Shortest distance from start
    int prev[MAX_NODES];     // Previous node in shortest path
    bool visited[MAX_NODES]; // Visited nodes tracker

    // Initialize arrays
    for (int i = 0; i < g->num_nodes; i++) {
        dist[i] = INF;
        prev[i] = -1;
        visited[i] = false;
    }

    dist[start] = 0;

    // Dijkstra's Main Loop
    for (int count = 0; count < g->num_nodes - 1; count++) {
        // 1. Pick the unvisited node with the minimum distance
        int min_dist = INF;
        int u = -1;

        for (int i = 0; i < g->num_nodes; i++) {
            if (!visited[i] && dist[i] < min_dist) {
                min_dist = dist[i];
                u = i;
            }
        }

        if (u == -1 || u == target) break; // Reached target or disconnected

        visited[u] = true;

        // 2. Update distance value of the adjacent vertices
        for (int v = 0; v < g->num_nodes; v++) {
            if (!visited[v] && g->adj_matrix[u][v] != INF) {
                int alt_dist = dist[u] + g->adj_matrix[u][v];
                if (alt_dist < dist[v]) {
                    dist[v] = alt_dist;
                    prev[v] = u; // Record path
                }
            }
        }
    }

    // --- Path Reconstruction & Output ---
    if (dist[target] == INF) {
        printf("No valid route from %s to %s.\n", start_name, target_name);
        return;
    }

    printf("\n--- Optimal Routing Path ---\n");
    printf("Source: %s | Target: %s\n", start_name, target_name);
    printf("Total Latency: %d ms\n", dist[target]);
    
    // Trace back the path using the 'prev' array
    int path[MAX_NODES];
    int path_len = 0;
    int curr = target;
    
    while (curr != -1) {
        path[path_len++] = curr;
        curr = prev[curr];
    }

    printf("Route: ");
    // Print in reverse order (from start to target)
    for (int i = path_len - 1; i >= 0; i--) {
        printf("%s", g->names[path[i]]);
        if (i > 0) printf(" -> ");
    }
    printf("\n----------------------------\n");
}

// --- Main Interface ---
int main() {
    NetworkGraph net;
    init_network(&net);

    // Hardcode Data from Question Description
    printf("Initializing Datacenter Network Topology...\n");
    add_link(&net, "S1", "S2", 8);
    add_link(&net, "S1", "S4", 20);
    add_link(&net, "S2", "S3", 7);
    add_link(&net, "S3", "S6", 12);
    add_link(&net, "S4", "S5", 4);
    add_link(&net, "S5", "S6", 6);
    add_link(&net, "S2", "X",  3);
    add_link(&net, "X",  "S5", 5);

    char start[NAME_LEN], target[NAME_LEN];
    int running = 1;

    while (running) {
        printf("\nEnter Source Server (e.g., S1) or 'exit' to quit: ");
        scanf("%s", start);
        if (strcmp(start, "exit") == 0) break;

        printf("Enter Target Server (e.g., S6): ");
        scanf("%s", target);

        find_shortest_path(&net, start, target);
    }

    printf("Routing simulator terminated.\n");
    return 0;
}
