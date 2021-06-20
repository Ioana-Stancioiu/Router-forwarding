#include <rtable.h>

#define INITIAL_CAPACITY 65000

int compare_prefixes(const void* a, const void* b) {
    struct route_table_entry* entry1 = (struct route_table_entry*)a;
    struct route_table_entry* entry2 = (struct route_table_entry*)b;
    if (ntohl(entry1->prefix) == ntohl(entry2->prefix)) {
        return (ntohl(entry1->mask) - ntohl(entry2->mask));
    }
    return (ntohl(entry1->prefix) - ntohl(entry2->prefix));
}

/**
 * @brief builds an entry for the routing table
 * with the information gathered from buffer
 */
struct route_table_entry create_entry(char* buffer) {
    struct route_table_entry entry;

    char buffer_info[4][16];

    char* token;
    int index = 0;
    token = strtok(buffer, " ");

    while (token != NULL) {
        memcpy(&buffer_info[index], token, strlen(token));
        buffer_info[index][strlen(token)] = '\0';
        index++;
        token = strtok(NULL, " ");
    }

    entry.prefix = inet_addr(buffer_info[0]);

    entry.next_hop = inet_addr(buffer_info[1]);

    entry.mask = inet_addr(buffer_info[2]);

    entry.interface = atoi(buffer_info[3]);

    return entry;
}

int read_table(char* file_name, struct route_table_entry** rtable) {
    FILE* input = fopen(file_name, "r");

    if (input == NULL) {
        printf("Error opening input file\n");
        exit(0);
    }

    *rtable = malloc(sizeof(struct route_table_entry) * INITIAL_CAPACITY);
    int capacity = INITIAL_CAPACITY;

    if (*rtable == NULL) {
        printf("Error allocating memory for routing table\n");
        exit(0);
    }

    int rtable_size = 0;

    char* buffer = NULL;
    size_t buffer_size = 0;
    int line_size = getline(&buffer, &buffer_size, input);

    while (line_size >= 0)
    {
        if (rtable_size == capacity) {
            capacity *= 2;
            *rtable = realloc(*rtable, capacity * sizeof(struct route_table_entry));

            if (*rtable == NULL) {
                printf("Reallocating memory failed\n");
                exit(0);
            }
        }

        (*rtable)[rtable_size++] = create_entry(buffer);
        line_size = getline(&buffer, &buffer_size, input);
    }

    free(buffer);
    fclose(input);

    //sort rtable by prefix
    qsort(*rtable, rtable_size, sizeof(struct route_table_entry), compare_prefixes);

    return rtable_size;
}


/**
 * @brief searches for best matching route in rtable
 * using binary search algorithm
 */
int binary_search(struct route_table_entry* rtable, int start, int stop,
                  int best_index, uint32_t dest_ip) {

    if (start > stop) {
        return best_index;
    }

    int mij = start + (stop - start) / 2;
    if ((rtable[mij].mask & dest_ip) == rtable[mij].prefix) {
        if ((best_index == -1) ||
            (ntohl(rtable[mij].mask) > ntohl(rtable[best_index].mask))) {
            return binary_search(rtable, mij + 1, stop, mij, dest_ip);
        } else {
            return binary_search(rtable, mij + 1, stop, best_index, dest_ip);
        }
    }

    if (ntohl(dest_ip & rtable[mij].mask) < ntohl(rtable[mij].prefix)) {
        return binary_search(rtable, start, mij - 1, best_index, dest_ip);
    }
    return binary_search(rtable, mij + 1, stop, best_index, dest_ip);
}

struct route_table_entry* get_best_route(struct route_table_entry* rtable,
                                        int rtable_size,
                                        uint32_t dest_ip) {

    int best_index = binary_search(rtable, 0, rtable_size - 1, -1, dest_ip);

    if (best_index == -1) {
        printf("Route doesn't exist\n");
        return NULL;
    }

    return &rtable[best_index];
}

