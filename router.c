#include <queue.h>
#include "skel.h"
#include "rtable.h"
#include "arptable.h"

int main(int argc, char *argv[])
{
	setvbuf(stdout,NULL,_IONBF,0);
	packet m;
	int rc;

	init(argc - 2, argv + 2);

	struct route_table_entry* rtable;
	int rtable_size = read_table(argv[1], &rtable);

	int arp_table_size = 0;
	struct arp_entry* arp_table;

	//queue for packets that have to be send
	queue pending_packets = queue_create();

	while (1) {
		rc = get_packet(&m);
		DIE(rc < 0, "get_message");

		struct ether_header *eth_hdr = (struct ether_header *)m.payload;
		struct iphdr *ip_hdr = (struct iphdr *)(m.payload + sizeof(struct ether_header));

		//address of interface on which the packet was send
		in_addr_t interface_addr = inet_addr(get_interface_ip(m.interface));

		//ICMP ECHO REQUEST
		if (ntohl(interface_addr) == ntohl(ip_hdr->daddr)) {
			struct icmphdr* icmp_hdr = parse_icmp(m.payload);
			if (icmp_hdr) {
				if (icmp_hdr->type == ICMP_ECHO) {
					send_icmp(ip_hdr->saddr, interface_addr,
							  eth_hdr->ether_dhost, eth_hdr->ether_shost,
							  ICMP_ECHOREPLY, 0, m.interface,
							  icmp_hdr->un.echo.id, icmp_hdr->un.echo.sequence);
					continue;
				}
			}
			continue;
		}

		//ARP
		struct arp_header* arp_hdr = parse_arp(m.payload);
		if (arp_hdr) {
			if (ntohl(arp_hdr->tpa) == ntohl(interface_addr)) {
				//ARP REQUEST
				if (ntohs(arp_hdr->op) == ARPOP_REQUEST) {
					//update dest and source MAC
					memcpy(eth_hdr->ether_dhost, arp_hdr->sha, ETH_ALEN);
					get_interface_mac(m.interface, eth_hdr->ether_shost);

					send_arp(arp_hdr->spa, interface_addr,
							 eth_hdr, m.interface, htons(ARPOP_REPLY));
					continue;
				}

			}

			//ARP REPLY
			if (ntohs(arp_hdr->op) == ARPOP_REPLY) {

				//send first packet from queue
				if (!queue_empty(pending_packets)) {
					packet* p = queue_deq(pending_packets);
					struct ether_header* eth_hdrp = (struct ether_header*)p->payload;

					//update dest MAC
					memcpy(eth_hdrp->ether_dhost, arp_hdr->sha, ETH_ALEN);
					//send packet
					send_packet(p->interface, p);
				}

				//ADD ENTRY TO ARP TABLE
				arp_table_size = add_arp_entry(&arp_table, arp_table_size,
												arp_hdr->spa, arp_hdr->sha);
				continue;
			}

			continue;
		}

		//CHECK TTL
		if (ip_hdr->ttl <= 1) {
			printf("TTL exceeded\n");
			send_icmp_error(ip_hdr->saddr, interface_addr,
							eth_hdr->ether_dhost, eth_hdr->ether_shost,
							ICMP_TIME_EXCEEDED, 0, m.interface);
			continue;
		}

		//CHECK CHECKSUM
		if (ip_checksum(ip_hdr, sizeof(struct iphdr)) != 0) {
			printf("Checksum incorrect\n");
			continue;
		}

		//UPDATE TTL AND CHECKSUM
		ip_hdr->ttl--;
		ip_hdr->check = 0;
		ip_hdr->check = ip_checksum(ip_hdr, sizeof(struct iphdr));


		//GET BEST ROUTE
		struct route_table_entry* best_route = get_best_route(rtable, rtable_size, ip_hdr->daddr);

		//TEST FOR DEST UNREACHABLE
		if (best_route == NULL) {
			printf("Destination unreachable\n");
			send_icmp_error(ip_hdr->saddr, interface_addr,
			      			eth_hdr->ether_dhost, eth_hdr->ether_shost,
							ICMP_DEST_UNREACH, 0, m.interface);
			continue;
		}

		//CHANGE INTERFACE
		m.interface = best_route->interface;

		//CHANGE MAC SOURCE
		get_interface_mac(m.interface, eth_hdr->ether_shost);

		//LOOK FOR ENTRY IN ARP TABLE (for dest mac)
		struct arp_entry* arp_entry = get_arp_entry(arp_table, arp_table_size, best_route->next_hop);

		//can't find mac dest in arp table
		if (arp_entry == NULL) {
			//send ARP REQUEST for mac dest
			printf("Send ARP REQUEST\n");

			//build ethernet header
			struct ether_header eheader;
			memcpy(eheader.ether_shost, eth_hdr->ether_shost, ETH_ALEN);
			hwaddr_aton("ff:ff:ff:ff:ff:ff", eheader.ether_dhost);

			eheader.ether_type = htons(ETH_P_ARP);

			send_arp(best_route->next_hop, inet_addr(get_interface_ip(m.interface)),
					 &eheader, m.interface, htons(ARPOP_REQUEST));

			//adding packet to queue
			packet p;
			memcpy(&p, &m, sizeof(m));
			queue_enq(pending_packets, &p);

			continue;
		}

		//CHANGE MAC DESTINATION
		memcpy(eth_hdr->ether_dhost, arp_entry->mac, ETH_ALEN);

		//ALL GOOD
		//send packet
		send_packet(m.interface, &m);
	}
}
