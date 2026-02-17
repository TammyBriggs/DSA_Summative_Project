#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 20
#define ID_LEN 10

typedef struct {
    char user_ids[MAX_USERS][ID_LEN];
    int adj_matrix[MAX_USERS][MAX_USERS];
    int num_users;
} SocialGraph;

// --- Graph Initialization ---
void init_graph(SocialGraph* g) {
    g->num_users = 0;
    for (int i = 0; i < MAX_USERS; i++) {
        for (int j = 0; j < MAX_USERS; j++) {
            g->adj_matrix[i][j] = 0;
        }
    }
}

// --- Helper: Get Index from ID ---
int get_user_index(SocialGraph* g, const char* id) {
    for (int i = 0; i < g->num_users; i++) {
        if (strcmp(g->user_ids[i], id) == 0) {
            return i;
        }
    }
    return -1; // Not found
}

// --- Dynamic Updates ---

// Add a user if they don't exist
int add_user(SocialGraph* g, const char* id) {
    int idx = get_user_index(g, id);
    if (idx != -1) return idx; // Already exists
    
    if (g->num_users >= MAX_USERS) {
        printf("Error: Max user limit reached.\n");
        return -1;
    }

    strcpy(g->user_ids[g->num_users], id);
    g->num_users++;
    return g->num_users - 1;
}

// Add a directed edge (interaction)
void add_interaction(SocialGraph* g, const char* from_id, const char* to_id) {
    int u = add_user(g, from_id); // Ensure users exist
    int v = add_user(g, to_id);

    if (u != -1 && v != -1) {
        g->adj_matrix[u][v] = 1; // Directed Edge: u -> v
        printf("Interaction Logged: %s -> %s\n", from_id, to_id);
    }
}

// Remove an interaction (Edge)
void remove_interaction(SocialGraph* g, const char* from_id, const char* to_id) {
    int u = get_user_index(g, from_id);
    int v = get_user_index(g, to_id);

    if (u != -1 && v != -1) {
        g->adj_matrix[u][v] = 0;
        printf("Interaction Removed: %s -> %s\n", from_id, to_id);
    } else {
        printf("Error: One or both users not found.\n");
    }
}

// Remove a user and all their associated interactions (Node)
void remove_user(SocialGraph* g, const char* id) {
    int k = get_user_index(g, id); // Find the index of the user to remove
    
    if (k == -1) {
        printf("Error: User %s not found.\n", id);
        return;
    }

    // Step 1: Shift User IDs array left to overwrite the user
    for (int i = k; i < g->num_users - 1; i++) {
        strcpy(g->user_ids[i], g->user_ids[i+1]);
    }

    // Step 2: Shift Rows up (Overwrite row k)
    // We iterate from the row we want to remove (k) to the end
    for (int i = k; i < g->num_users - 1; i++) {
        for (int j = 0; j < g->num_users; j++) {
            g->adj_matrix[i][j] = g->adj_matrix[i+1][j];
        }
    }

    // Step 3: Shift Columns left (Overwrite column k)
    // We do this for every row in the matrix
    for (int i = 0; i < g->num_users; i++) {
        for (int j = k; j < g->num_users - 1; j++) {
            g->adj_matrix[i][j] = g->adj_matrix[i][j+1];
        }
    }

    g->num_users--; // Decrease the total count
    printf("User %s and all their interactions have been removed.\n", id);
}

// --- Query Functions ---

void query_user(SocialGraph* g, const char* id) {
    int idx = get_user_index(g, id);
    if (idx == -1) {
        printf("User %s not found in the system.\n", id);
        return;
    }

    printf("\n--- Analysis for %s ---\n", id);

    // 1. Outgoing (Who they interact with) -> Scan Row
    printf("Interacts WITH (Outgoing): ");
    int found_out = 0;
    for (int j = 0; j < g->num_users; j++) {
        if (g->adj_matrix[idx][j] == 1) {
            printf("%s, ", g->user_ids[j]);
            found_out = 1;
        }
    }
    if (!found_out) printf("None");
    printf("\n");

    // 2. Incoming (Who interacts with them) -> Scan Column
    printf("Interacted BY (Incoming):  ");
    int found_in = 0;
    for (int i = 0; i < g->num_users; i++) {
        if (g->adj_matrix[i][idx] == 1) {
            printf("%s, ", g->user_ids[i]);
            found_in = 1;
        }
    }
    if (!found_in) printf("None");
    printf("\n--------------------------\n");
}

// --- Matrix Display ---
void print_adjacency_matrix(SocialGraph* g) {
    printf("\nAdjacency Matrix:\n      ");
    for (int i = 0; i < g->num_users; i++) {
        printf("%s ", g->user_ids[i]);
    }
    printf("\n");

    for (int i = 0; i < g->num_users; i++) {
        printf("%s  ", g->user_ids[i]);
        for (int j = 0; j < g->num_users; j++) {
            printf("  %d  ", g->adj_matrix[i][j]);
        }
        printf("\n");
    }
}

// --- Main Interface ---

int main() {
    SocialGraph graph;
    init_graph(&graph);

    // 1. Hardcode Data from Question Image
    printf("Initializing Network Data...\n");
    add_interaction(&graph, "U101", "U102");
    add_interaction(&graph, "U101", "U103");
    add_interaction(&graph, "U102", "U104");
    add_interaction(&graph, "U103", "U105");
    add_interaction(&graph, "U104", "U105");
    add_interaction(&graph, "U104", "U106");
    add_interaction(&graph, "U105", "U107");
    add_interaction(&graph, "U106", "U108");

    int choice;
    char id1[ID_LEN], id2[ID_LEN];

    while(1) {
        printf("\n1. Show Matrix\n2. Query User\n3. Add Interaction\n4. Remove Interaction\n5. Remove User\n6. Exit\nSelect: ");
        if (scanf("%d", &choice) != 1) break;

        switch(choice) {
            case 1:
                print_adjacency_matrix(&graph);
                break;
            case 2:
                printf("Enter User ID (e.g., U103): ");
                scanf("%s", id1);
                query_user(&graph, id1);
                break;
            case 3:
                printf("Enter From_ID To_ID: ");
                scanf("%s %s", id1, id2);
                add_interaction(&graph, id1, id2);
                break;
            case 4:
                printf("Enter From_ID To_ID to remove: ");
                scanf("%s %s", id1, id2);
                remove_interaction(&graph, id1, id2);
                break;
            case 5:
                printf("Enter User ID to delete: ");
                scanf("%s", id1);
                remove_user(&graph, id1);
                break;
            case 6:
                printf("Exiting tool.\n");
                return 0;
            default:
                printf("Invalid choice.\n");
        }
    }
    return 0;
}
