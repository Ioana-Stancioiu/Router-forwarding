#include<arptable.h>

int table_size = 10;

struct arp_entry* get_arp_entry(struct arp_entry* arp_table,
                                int arp_table_size, uint32_t ip) {

    for (int i = 0; i < arp_table_size; i++) {
        if (arp_table[i].ip == ip) {
            return &arp_table[i];
        }
    }

    printf("No matching route found\n");
    return NULL;
}

int add_arp_entry(struct arp_entry** arp_table, int current_size, uint32_t ip, uint8_t* mac) {
    if (*arp_table == NULL) {
        *arp_table = malloc(table_size * sizeof(struct arp_entry));
        current_size = 0;

        if (*arp_table == NULL) {
            printf("ARP table memory allocation failed\n");
            exit(0);
        }
    }

    if (current_size == table_size) {
        table_size *= 2;
        *arp_table = realloc(*arp_table, table_size * sizeof(struct arp_entry));

        if (*arp_table == NULL) {
            printf("ARP table memory reallocation failed\n");
            exit(0);
        }
    }

    for (int i = 0; i < current_size; i++) {
        if ((*arp_table)[i].ip == ip) {
            return current_size;
        }
    }

    (*arp_table)[current_size].ip = ip;
    memcpy((*arp_table)[current_size].mac, mac, ETH_ALEN);
    current_size++;

    return current_size;
}
