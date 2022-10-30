#include <net/stack.h>
#include <memory/vmm.h>
#include <string.h>

#include <net/etherframe.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/dhcp.h>

#include <stdio.h>

void load_network_stack(nic_driver_t* nic) {
	ip_u my_ip = {.ip_p = {10, 0, 2, 15}};
	ip_u gateway_ip = {.ip_p = {10, 0, 2, 2}};
	ip_u subnet_mask = {.ip_p = {255, 255, 255, 0}};

	network_stack_t* stack = vmm_alloc(sizeof(network_stack_t) / 0x1000 + 1);
	memset(stack, 0, sizeof(network_stack_t));

	nic->driver.driver_specific_data = stack;
	stack->driver = nic;

	etherframe_init(stack);
	arp_init(stack);
	ipv4_init(stack, (ip_u) {.ip = 0xffffffff}, (ip_u) {.ip = 0xffffffff});
	icmp_init(stack);
	udp_init(stack);
    dhcp_init(stack);

    dhcp_request(stack);

	nic->ip = stack->dhcp->ip;
    stack->ipv4->gateway_ip = stack->dhcp->gateway;
    stack->ipv4->subnet_mask = stack->dhcp->subnet;

	arp_broadcast_mac(stack, gateway_ip);

	bool res = icmp_send_echo_reqest_and_wait(stack, (ip_u) {.ip_p = {10, 0, 2, 2}});
	debugf("icmp: %s", res ? "true" : "false");

	char buf[] = "hello world";
	udp_socket_t* sock = udp_connect(stack, gateway_ip, 9999);
	udp_socket_send(sock, buf, sizeof(buf));
	udp_socket_disconnect(sock);
}
