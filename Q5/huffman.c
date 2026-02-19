#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TREE_HT 256

// --- Data Structures ---
typedef struct MinHeapNode {
    char data;
    unsigned freq;
    struct MinHeapNode *left, *right;
} MinHeapNode;

typedef struct MinHeap {
    unsigned size;
    unsigned capacity;
    MinHeapNode** array;
} MinHeap;

// Global array to store generated codes
char huffman_codes[256][MAX_TREE_HT];

// --- Min-Heap Utilities ---
MinHeapNode* newNode(char data, unsigned freq) {
    MinHeapNode* temp = (MinHeapNode*)malloc(sizeof(MinHeapNode));
    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;
    return temp;
}

MinHeap* createMinHeap(unsigned capacity) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (MinHeapNode**)malloc(minHeap->capacity * sizeof(MinHeapNode*));
    return minHeap;
}

void swapMinHeapNode(MinHeapNode** a, MinHeapNode** b) {
    MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;
    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx) {
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

MinHeapNode* extractMin(MinHeap* minHeap) {
    MinHeapNode* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    --minHeap->size;
    minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(MinHeap* minHeap, MinHeapNode* minHeapNode) {
    ++minHeap->size;
    int i = minHeap->size - 1;
    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->array[i] = minHeapNode;
}

// --- Huffman Tree Generation ---
MinHeapNode* buildHuffmanTree(unsigned freq[], int unique_chars) {
    MinHeapNode *left, *right, *top;
    MinHeap* minHeap = createMinHeap(unique_chars);

    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            minHeap->array[minHeap->size++] = newNode((char)i, freq[i]);
        }
    }
    
    // Build heap
    for (int i = (minHeap->size - 2) / 2; i >= 0; --i) minHeapify(minHeap, i);

    while (minHeap->size != 1) {
        left = extractMin(minHeap);
        right = extractMin(minHeap);
        top = newNode('$', left->freq + right->freq); // '$' is internal node marker
        top->left = left;
        top->right = right;
        insertMinHeap(minHeap, top);
    }
    return extractMin(minHeap);
}

void storeCodes(MinHeapNode* root, int arr[], int top) {
    if (root->left) {
        arr[top] = 0;
        storeCodes(root->left, arr, top + 1);
    }
    if (root->right) {
        arr[top] = 1;
        storeCodes(root->right, arr, top + 1);
    }
    if (!(root->left) && !(root->right)) {
        for (int i = 0; i < top; ++i) {
            huffman_codes[(unsigned char)root->data][i] = arr[i] ? '1' : '0';
        }
        huffman_codes[(unsigned char)root->data][top] = '\0';
    }
}

// --- Compression Module ---
void compressFile(const char* in_filename, const char* out_filename) {
    FILE* in = fopen(in_filename, "rb");
    if (!in) { printf("Error: Cannot open %s\n", in_filename); return; }

    // 1. Frequency Analysis
    unsigned freq[256] = {0};
    int ch;
    unsigned total_chars = 0;
    int unique_chars = 0;

    while ((ch = fgetc(in)) != EOF) {
        if (freq[ch] == 0) unique_chars++;
        freq[ch]++;
        total_chars++;
    }
    rewind(in);

    // 2. Build Tree & Codes
    MinHeapNode* root = buildHuffmanTree(freq, unique_chars);
    int arr[MAX_TREE_HT], top = 0;
    memset(huffman_codes, 0, sizeof(huffman_codes));
    storeCodes(root, arr, top);

    // 3. Write Compressed File
    FILE* out = fopen(out_filename, "wb");
    
    // Header: Write frequencies and total char count so we can rebuild the tree
    fwrite(freq, sizeof(unsigned), 256, out);
    fwrite(&total_chars, sizeof(unsigned), 1, out);

    // Bit Packing Loop
    unsigned char buffer = 0;
    int bit_count = 0;
    
    while ((ch = fgetc(in)) != EOF) {
        char* code = huffman_codes[(unsigned char)ch];
        for (int i = 0; code[i] != '\0'; i++) {
            buffer = buffer << 1;
            if (code[i] == '1') buffer = buffer | 1;
            bit_count++;

            if (bit_count == 8) {
                fputc(buffer, out);
                buffer = 0;
                bit_count = 0;
            }
        }
    }
    // Flush remaining bits
    if (bit_count > 0) {
        buffer = buffer << (8 - bit_count); // pad with zeros
        fputc(buffer, out);
    }

    // Get File Sizes
    fseek(in, 0L, SEEK_END);
    long in_size = ftell(in);
    fseek(out, 0L, SEEK_END);
    long out_size = ftell(out);

    printf("\n--- Compression Summary ---\n");
    printf("Original File: %ld bytes\n", in_size);
    printf("Compressed File: %ld bytes\n", out_size);
    
    fclose(in);
    fclose(out);
}

// --- Decompression Module ---
void decompressFile(const char* in_filename, const char* out_filename) {
    FILE* in = fopen(in_filename, "rb");
    if (!in) { printf("Error: Cannot open %s\n", in_filename); return; }

    // 1. Read Header & Rebuild Tree
    unsigned freq[256];
    unsigned total_chars;
    fread(freq, sizeof(unsigned), 256, in);
    fread(&total_chars, sizeof(unsigned), 1, in);

    int unique_chars = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) unique_chars++;
    }

    MinHeapNode* root = buildHuffmanTree(freq, unique_chars);

    // 2. Decode Bits
    FILE* out = fopen(out_filename, "wb");
    MinHeapNode* current = root;
    int ch;
    unsigned chars_written = 0;

    while ((ch = fgetc(in)) != EOF && chars_written < total_chars) {
        for (int i = 7; i >= 0; i--) {
            int bit = (ch >> i) & 1;
            
            if (bit == 0) current = current->left;
            else current = current->right;

            if (current->left == NULL && current->right == NULL) {
                fputc(current->data, out);
                chars_written++;
                current = root; // Reset to top of tree
                if (chars_written == total_chars) break;
            }
        }
    }

    printf("Decompression complete. Output saved to %s\n", out_filename);
    fclose(in);
    fclose(out);
}

// --- Main Interface ---
int main() {
    int choice;
    char infile[100], outfile[100];

    while(1) {
        printf("\n--- Log Compression Utility ---\n");
        printf("1. Compress machine log\n2. Decompress archive\n3. Exit\nSelect: ");
        if (scanf("%d", &choice) != 1) break;

        switch(choice) {
            case 1:
                printf("Enter filename to compress (e.g., machine.log): ");
                scanf("%s", infile);
                compressFile(infile, "compressed.log");
                break;
            case 2:
                decompressFile("compressed.log", "decompressed.log");
                break;
            case 3:
                return 0;
            default:
                printf("Invalid choice.\n");
        }
    }
    return 0;
}
