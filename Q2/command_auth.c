#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CMD_LEN 50
#define MAX_COMMANDS 40
#define TYPO_THRESHOLD 3

// --- Data Structures ---
typedef struct TreeNode {
    char command[MAX_CMD_LEN];
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

// --- Helper: Levenshtein Distance (for typo suggestions) ---
// Calculates the minimum number of single-character edits required to change s1 into s2
int calculate_edit_distance(const char* s1, const char* s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    int matrix[len1 + 1][len2 + 1];

    for (int i = 0; i <= len1; i++) matrix[i][0] = i;
    for (int j = 0; j <= len2; j++) matrix[0][j] = j;

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            int del = matrix[i - 1][j] + 1;
            int ins = matrix[i][j - 1] + 1;
            int sub = matrix[i - 1][j - 1] + cost;
            
            // Find minimum of del, ins, sub
            int min = del < ins ? del : ins;
            matrix[i][j] = min < sub ? min : sub;
        }
    }
    return matrix[len1][len2];
}

// --- Core BST Operations ---

// Create a new BST node
TreeNode* create_node(const char* cmd) {
    TreeNode* new_node = (TreeNode*)malloc(sizeof(TreeNode));
    if (new_node) {
        strncpy(new_node->command, cmd, MAX_CMD_LEN);
        new_node->command[MAX_CMD_LEN - 1] = '\0'; // Ensure null-termination
        new_node->left = NULL;
        new_node->right = NULL;
    }
    return new_node;
}

// Insert a command into the BST
TreeNode* insert_command(TreeNode* root, const char* cmd) {
    if (root == NULL) return create_node(cmd);

    int cmp = strcmp(cmd, root->command);
    if (cmp < 0) {
        root->left = insert_command(root->left, cmd);
    } else if (cmp > 0) {
        root->right = insert_command(root->right, cmd);
    }
    return root; // Exact duplicates are ignored
}

// Search for an exact match in the BST
int search_exact(TreeNode* root, const char* cmd) {
    if (root == NULL) return 0; // Not found

    int cmp = strcmp(cmd, root->command);
    if (cmp == 0) return 1; // Exact match found
    if (cmp < 0) return search_exact(root->left, cmd);
    return search_exact(root->right, cmd);
}

// Traverse the BST to find the closest matching string (for typos)
void find_closest_match(TreeNode* root, const char* input_cmd, char* best_match, int* min_dist) {
    if (root == NULL) return;

    int current_dist = calculate_edit_distance(input_cmd, root->command);
    
    if (current_dist < *min_dist) {
        *min_dist = current_dist;
        strcpy(best_match, root->command);
    }

    find_closest_match(root->left, input_cmd, best_match, min_dist);
    find_closest_match(root->right, input_cmd, best_match, min_dist);
}

// Free the BST memory (Post-order traversal)
void free_tree(TreeNode* root) {
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

// --- File I/O Operations ---

// Load commands from a file into the BST
TreeNode* load_approved_commands(const char* filename, int* count) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open %s. Ensure the file exists.\n", filename);
        return NULL;
    }

    TreeNode* root = NULL;
    char buffer[MAX_CMD_LEN];
    *count = 0;

    while (fgets(buffer, sizeof(buffer), file) && *count < MAX_COMMANDS) {
        // Strip newline characters
        buffer[strcspn(buffer, "\r\n")] = 0;
        
        if (strlen(buffer) > 0) {
            root = insert_command(root, buffer);
            (*count)++;
        }
    }

    fclose(file);
    printf(">> Successfully loaded %d approved commands.\n", *count);
    return root;
}

// Log unrecognized commands
void log_unrecognized(const char* cmd) {
    FILE* file = fopen("unrecognized.log", "a");
    if (file) {
        fprintf(file, "REJECTED: %s\n", cmd);
        fclose(file);
    } else {
        printf("Error: Could not write to log file.\n");
    }
}

// --- Main Interface ---

int main() {
    TreeNode* root = NULL;
    int command_count = 0;
    
    printf("--- Industrial Control Terminal Initialization ---\n");
    root = load_approved_commands("approved_commands.txt", &command_count);
    
    if (root == NULL) {
        printf("System halted. Missing configuration.\n");
        return 1;
    }

    char input[MAX_CMD_LEN];
    
    while (1) {
        printf("\nTerminal> ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        
        // Strip newline
        input[strcspn(input, "\r\n")] = 0;

        // Handle Exit gracefully
        if (strcmp(input, "EXIT_TERMINAL") == 0) {
            printf("Shutting down terminal... cleaning memory.\n");
            break;
        }

        if (strlen(input) == 0) continue;

        // 1. Check for Exact Match
        if (search_exact(root, input)) {
            printf("[SUCCESS] Command '%s' Executed.\n", input);
            continue;
        }

        // 2. Exact match failed. Check for Minor Typo
        char best_match[MAX_CMD_LEN] = "";
        int min_dist = 9999; // Initialize with a high number
        
        find_closest_match(root, input, best_match, &min_dist);

        if (min_dist <= TYPO_THRESHOLD) {
            printf("[ERROR] Unrecognized command. Did you mean '%s'?\n", best_match);
        } else {
            // 3. Completely unrecognized
            printf("[SECURITY ALERT] Unrecognized command rejected and logged.\n");
            log_unrecognized(input);
        }
    }

    // Cleanup
    free_tree(root);
    return 0;
}
