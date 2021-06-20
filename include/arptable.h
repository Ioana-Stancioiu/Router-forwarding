#pragma once
#include "skel.h"
#include <stdio.h>
#include <unistd.h>

struct arp_entry {
	uint32_t ip;
	uint8_t mac[6];
} __attribute__((packed));

/**
 * @param arp_table
 * @param arp_table_size
 * @param ip to look for
 * @return a pointer to the best matching route in the arp table
 * or NULL if no route is found
 */
struct arp_entry* get_arp_entry(struct arp_entry* arp_table, int arp_table_size, uint32_t ip);

/**
 * @brief adds a new entry to the arp table if it doesn't exist already
 * @return new size of arp table
 */
int add_arp_entry(struct arp_entry** arp_table, int current_size, uint32_t ip, uint8_t* mac);
