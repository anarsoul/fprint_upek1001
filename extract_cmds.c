/*
 * fprint_upek1001 driver prototype helper
 * Copyright (c) 2013 Vasily Khoruzhick <anarsoul@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdint.h>
#include <linux/usb/ch9.h>

#include "bsapi_capture_init_separated.h"
#include "packets_new.h"

#define ARRAY_SIZE(a) (sizeof(a) / (sizeof(*a)))

void handle_control(const unsigned char *packet)
{
	int i;
	struct usb_ctrlrequest *req = (struct usb_ctrlrequest *)(packet + 0x28);
	//printf("{\n\t.reg = 0x%.2x,\n\t.len = 0x%.2x,\n\t.val = { ", req->wIndex, req->wLength);
	for (i = 0; i < req->wLength; i++) {
		printf("{ 0x%.2x, 0x%.2x }, ", req->wIndex + i, packet[0x40 + i]);
		//printf("0x%.2x%s", packet[0x40 + i], i != (req->wLength - 1) ? ", " : " }\n");
	}
	//printf("},\n");
	printf("\n");
}

void handle_bulk(const unsigned char *packet)
{
	uint32_t *bulk_len = (uint32_t *)(packet + 0x24);
	printf("\t/* bulk of size %d */\n", *bulk_len);
}

int main(int argc, char *argv[])
{
	int i;
	for (i = 0; i < ARRAY_SIZE(packets); i++) {
		switch (packets[i][9]) {
		/* Control transfer */
		case 0x02:
			handle_control(packets[i]);
		break;
		/* Bulk transfer */
		case 0x03:
			handle_bulk(packets[i]);
		break;
		default:
		printf("Unknown transfer!!!\n");
		break;
		}
	}
}
