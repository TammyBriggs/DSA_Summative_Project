#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- Configuration & Constants ---
#define MAX_EVENTS 20

// --- Data Structures ---
typedef enum {
    POWER_CONSUMPTION,
    VOLTAGE_LEVEL,
    FREQUENCY_STABILITY,
    FAULT_ALERT
} EventType;

typedef struct {
    int id;
    EventType type;
    float value;
    time_t timestamp;
} MeterEvent;

typedef struct Node {
    MeterEvent data;
    struct Node* next;
    struct Node* prev;
} Node;

typedef struct {
    Node* head;         // Oldest event
    Node* tail;         // Newest event
    Node* cursor;       // Current view position
    int count;
    int live_mode;      // 1 = ON, 0 = OFF
    int next_id;        // Auto-incrementing ID
} EventLog;

// --- Helper Functions ---

// Get string representation of event type
const char* get_event_type_str(EventType type) {
    switch (type) {
        case POWER_CONSUMPTION: return "PWR_CONS";
        case VOLTAGE_LEVEL: return "VOLT_LVL";
        case FREQUENCY_STABILITY: return "FREQ_STB";
        case FAULT_ALERT: return "FAULT_ALRT";
        default: return "UNKNOWN";
    }
}

// Display a single event
void print_event(Node* node, const char* label) {
    if (!node) {
        printf("[%s] No event selected.\n", label);
        return;
    }
    struct tm* tm_info = localtime(&node->data.timestamp);
    char time_buffer[26];
    strftime(time_buffer, 26, "%H:%M:%S", tm_info);

    printf("[%s] ID:%03d | Time:%s | Type:%s | Val:%.2f\n", 
           label, node->data.id, time_buffer, 
           get_event_type_str(node->data.type), node->data.value);
}

// --- Core DLL Operations ---

// Initialize the system
void init_log(EventLog* log) {
    log->head = NULL;
    log->tail = NULL;
    log->cursor = NULL;
    log->count = 0;
    log->live_mode = 0;
    log->next_id = 101; // Starting ID
    printf("System Initialized. Buffer size: %d\n", MAX_EVENTS);
}

// Remove the oldest event (Head)
void remove_oldest(EventLog* log) {
    if (log->head == NULL) return;

    Node* temp = log->head;
    
    // If cursor is pointing to the node we are deleting, move it to the next one
    if (log->cursor == temp) {
        log->cursor = temp->next;
        printf("<!> Oldest event removed. Cursor adjusted.\n");
    }

    if (log->head == log->tail) {
        // Only one item in list
        log->head = NULL;
        log->tail = NULL;
    } else {
        log->head = log->head->next;
        log->head->prev = NULL;
    }

    free(temp);
    log->count--;
}

// Add a new event to the list
void add_event(EventLog* log, EventType type, float value) {
    // 1. Enforce Memory Constraint
    if (log->count >= MAX_EVENTS) {
        remove_oldest(log);
    }

    // 2. Create New Node
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) {
        printf("Error: Memory allocation failed.\n");
        return;
    }

    new_node->data.id = log->next_id++;
    new_node->data.type = type;
    new_node->data.value = value;
    new_node->data.timestamp = time(NULL);
    new_node->next = NULL;

    // 3. Link into List
    if (log->count == 0) {
        new_node->prev = NULL;
        log->head = new_node;
        log->tail = new_node;
        log->cursor = new_node; // Cursor always starts at oldest (head) on init
    } else {
        new_node->prev = log->tail;
        log->tail->next = new_node;
        log->tail = new_node;
    }

    log->count++;

    // 4. Live Mode Handling
    if (log->live_mode) {
        print_event(new_node, "LIVE LOG");
    }
}

// Clear all memory
void clear_log(EventLog* log) {
    Node* current = log->head;
    while (current != NULL) {
        Node* next = current->next;
        free(current);
        current = next;
    }
    log->head = NULL;
    log->tail = NULL;
    log->cursor = NULL;
    log->count = 0;
    printf("Memory Cleared.\n");
}

// --- Simulation Helper ---
// Simulates a sensor reading
void simulate_hardware_event(EventLog* log) {
    int r = rand() % 4;
    float val = (float)(rand() % 100) + ((rand() % 10) * 0.1);
    add_event(log, (EventType)r, val);
}

// --- Main Interface ---

int main() {
    srand(time(NULL));
    EventLog log;
    init_log(&log);

    char command;
    int running = 1;

    // Pre-populate a few events for testing
    printf("Booting firmware... detecting initial signals...\n");
    simulate_hardware_event(&log);
    simulate_hardware_event(&log);
    simulate_hardware_event(&log);

    while (running) {
        printf("\n--- Energy Gateway (Events: %d/%d) [Live: %s] ---\n", 
               log.count, MAX_EVENTS, log.live_mode ? "ON" : "OFF");
        
        // Show current cursor position
        print_event(log.cursor, "CURSOR");

        printf("Commands: (n)ext, (p)rev, (r)esume live, (h)alt live, (x)it, (c)lear, (+)sim event: ");
        scanf(" %c", &command);

        switch (command) {
            case 'n': // Next (Newer)
                if (log.cursor && log.cursor->next) {
                    log.cursor = log.cursor->next;
                } else {
                    printf(">> End of history.\n");
                }
                break;

            case 'p': // Previous (Older)
                if (log.cursor && log.cursor->prev) {
                    log.cursor = log.cursor->prev;
                } else {
                    printf(">> Start of history.\n");
                }
                break;

            case 'r': // Resume Live Display
                log.live_mode = 1;
                printf(">> Live display STARTED.\n");
                break;

            case 'h': // Halt Live Display
                log.live_mode = 0;
                printf(">> Live display PAUSED (events still collecting).\n");
                break;
            
            case 'c': // Clear
                clear_log(&log);
                break;

            case 'x': // Exit
                printf(">> Saving state... System Shutdown.\n");
                clear_log(&log); // Cleanup before exit
                running = 0;
                break;
            
            case '+': // HIDDEN/TEST COMMAND: Manually trigger a sensor event
                printf(">> Sensor signal received...\n");
                simulate_hardware_event(&log);
                break;

            default:
                printf(">> Invalid Command.\n");
        }
    }
    return 0;
}
