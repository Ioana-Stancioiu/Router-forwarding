#pragma once
#include <stdio.h>
#include <unistd.h>
#include "skel.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct route_table_entry {
	uint32_t prefix;
	uint32_t next_hop;
	uint32_t mask;
	int interface;
} __attribute__((packed));

/**
 * @brief parses the routing table txt file and returns contents and size
 *
 * @param file_name
 * @param rtable array containing routing table entries
 * @return size of routing table array
 */
int read_table(char* file_name, struct route_table_entry** rtabel);

/**
 * @return pointer to the best matching route for dest_ip
 * or NULL if no route is found
 */
struct route_table_entry* get_best_route(struct route_table_entry* rtable, int rtable_size, uint32_t dest_ip);
