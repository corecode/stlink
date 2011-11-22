/* -*- tab-width:8 -*- */

/*
 Copyright (C)  2011 Peter Zotov <whitequark@whitequark.org>
 Use of this source code is governed by a BSD-style
 license that can be found in the LICENSE file.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#if defined(CONFIG_WIN32) && (CONFIG_WIN32 +1 > 1)
  #include <windows.h>
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <sys/socket.h>
  #include <sys/select.h>
#endif

static const char hex[] = "0123456789abcdef";

int gdb_send_packet(int fd, char* data) {
	unsigned length = strlen(data) + 5;
	char* packet = malloc(length); /* '$' data (hex) '#' cksum (hex) */

	memset(packet, 0, length);

	packet[0] = '$';

	uint8_t cksum = 0;
	for(int i = 0; i < strlen(data); i++) {
		packet[i + 1] = data[i];
		cksum += data[i];
	}

	packet[length - 4] = '#';
	packet[length - 3] = hex[cksum >> 4];
	packet[length - 2] = hex[cksum & 0xf];

	while(1) {
		if(send(fd, packet, length, 0) != length) {
			free(packet);
			return -2;
		}

		char ack;
		if(recv(fd, &ack, 1, 0) != 1) {
			free(packet);
			return -2;
		}

		if(ack == '+') {
			free(packet);
			return 0;
		}
	}
}

#define ALLOC_STEP 1024

int gdb_recv_packet(int fd, char** buffer) {
	unsigned packet_size = ALLOC_STEP + 1, packet_idx = 0;
	uint8_t cksum = 0;
	char recv_cksum[3] = {0};
	char* packet_buffer = malloc(packet_size);
	unsigned state;

start:
	state = 0;
	/*
	 * 0: waiting $
	 * 1: data, waiting #
	 * 2: cksum 1
	 * 3: cksum 2
	 * 4: fin
	 */

	char c;
	while(state != 4) {
		if(recv(fd, &c, 1, 0) != 1) {
			return -2;
		}

		switch(state) {
		case 0:
			if(c != '$') {
				// ignore
			} else {
				state = 1;
			}
			break;

		case 1:
			if(c == '#') {
				state = 2;
			} else {
				packet_buffer[packet_idx++] = c;
				cksum += c;

				if(packet_idx == packet_size) {
					packet_size += ALLOC_STEP;
					packet_buffer = realloc(packet_buffer, packet_size);
				}
			}
			break;

		case 2:
			recv_cksum[0] = c;
			state = 3;
			break;

		case 3:
			recv_cksum[1] = c;
			state = 4;
			break;
		}
	}

	uint8_t recv_cksum_int = strtoul(recv_cksum, NULL, 16);
	if(recv_cksum_int != cksum) {
		char nack = '-';
		if(send(fd, &nack, 1, 0) != 1) {
			return -2;
		}

		goto start;
	} else {
		char ack = '+';
		if(send(fd, &ack, 1, 0) != 1) {
			return -2;
		}
	}

	packet_buffer[packet_idx] = 0;
	*buffer = packet_buffer;

	return packet_idx;
}

// Here we skip any characters which are not \x03, GDB interrupt.
// As we use the mode with ACK, in a (very unlikely) situation of a packet
// lost because of this skipping, it will be resent anyway.
int gdb_check_for_interrupt(int fd) {
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	int retval = select(1, &rfds, NULL, NULL, &tv);
	FD_CLR(fd, &rfds);

	if (retval > 0) {
		char c;
		if (recv(fd, &c, 1, 0) != 1)
			return -2;

		if(c == '\x03') // ^C
			return 1;
	}

	return 0;
}
