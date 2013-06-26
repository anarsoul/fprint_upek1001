/*
 * fprint_upek1001 driver prototype
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
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <libusb.h>

struct upek1001_regwrite {
	uint8_t addr;
	uint8_t val;
};

#include "upek1001.h"

#define EP_IN (1 | LIBUSB_ENDPOINT_IN)
#define EP_OUT (2 | LIBUSB_ENDPOINT_OUT)
#define EP_INTR (3 | LIBUSB_ENDPOINT_IN)
#define BULK_TIMEOUT 4000

#define ARRAY_SIZE(a) (sizeof(a) / (sizeof(*a)))

int aborted = 0;

static void __msg(FILE *stream, const char *msg, va_list args)
{
	vfprintf(stream, msg, args);
	fprintf(stream, "\n");
}

static void die(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	__msg(stderr, msg, args);
	va_end(args);
	exit(1);
}

static void msg(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	__msg(stdout, msg, args);
	va_end(args);
}

static void msg_err(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	__msg(stderr, msg, args);
	va_end(args);
}

static void sighandler(int num)
{
	msg("got signal %d\n", num);
	aborted = 1;
}

static void write_file(FILE *out, unsigned char *buf, unsigned int len)
{
	int seq;
	/* 2 bytes seq + 62 bytes data */
	while (len) {
		seq = buf[0] * 256 + buf[1];
		//msg("seq: %d\n", seq);
		buf += 2;
		len -= 2;
		fwrite(buf, 62, 1, out);
		buf += 62;
		len -= 62;
	}
}

static int upek1001_reg_write(libusb_device_handle *h, uint16_t reg, unsigned char data)
{
	int r;
	unsigned char cmd_data[1];

	cmd_data[0] = data;

	r = libusb_control_transfer(h, 0x40, 0x0c, 0x0000, reg, cmd_data, 1, BULK_TIMEOUT);

	return 0;
}

static int upek1001_reg_read(libusb_device_handle *h, uint16_t reg, unsigned char *data, unsigned int len)
{
	int r;

	r = libusb_control_transfer(h, 0xc0, 0x0c, 0x0000, reg, data, len, BULK_TIMEOUT);

	return r;
}

static int upek1001_read_bytes(libusb_device_handle *h, unsigned char *buf, unsigned int bytes, int *actual_len)
{
	int r;

	*actual_len = 0;

	r = libusb_bulk_transfer(h, EP_IN, buf, bytes, actual_len, BULK_TIMEOUT);

	return r;
}

static int upek1001_test(libusb_device_handle *h)
{
	int r, i, idx = 0;
	int actual_len;
	FILE *out;
	unsigned char buf[8192];

	/* INIT 1 */
	for (i = 0; i < ARRAY_SIZE(init_1); i++) {
		r = upek1001_reg_write(h, init_1[i].addr, init_1[i].val);
		if (r) {
			msg("Init_1 err %d at %d\n", r, i);
			break;
		}
	}
	msg("Reading bytes...\n");
	r = upek1001_read_bytes(h, buf, 64, &actual_len);
	msg("r = %d, actual: %d\n", r, actual_len);

	/* INIT 2 */
	for (i = 0; i < ARRAY_SIZE(init_2); i++) {
		r = upek1001_reg_write(h, init_2[i].addr, init_2[i].val);
		if (r) {
			msg("Init_2 err %d at %d\n", r, i);
			break;
		}
	}
	msg("Reading bytes...\n");
	r = upek1001_read_bytes(h, buf, 64, &actual_len);
	msg("r = %d, actual: %d\n", r, actual_len);

	/* INIT 3 */
	for (i = 0; i < ARRAY_SIZE(init_3); i++) {
		r = upek1001_reg_write(h, init_3[i].addr, init_3[i].val);
		if (r) {
			msg("Init_3 err %d at %d\n", r, i);
			break;
		}
	}
	msg("Reading bytes...\n");
	r = upek1001_read_bytes(h, buf, 512, &actual_len);
	msg("r = %d, actual: %d\n", r, actual_len);

	/* INIT 4 */
	for (i = 0; i < ARRAY_SIZE(init_4); i++) {
		r = upek1001_reg_write(h, init_4[i].addr, init_4[i].val);
		if (r) {
			msg("Init_4 err %d at %d\n", r, i);
			break;
		}
	}
	msg("Reading bytes...\n");
	r = upek1001_read_bytes(h, buf, 512, &actual_len);
	msg("r = %d, actual: %d\n", r, actual_len);

	/* INIT 5 */
	for (i = 0; i < ARRAY_SIZE(init_5); i++) {
		r = upek1001_reg_write(h, init_5[i].addr, init_5[i].val);
		if (r) {
			msg("Init_5 err %d at %d\n", r, i);
			break;
		}
	}

	msg("Init done, starting capture\n");

	/* CAPTURE 1 */
	for (i = 0; i < ARRAY_SIZE(capture_1); i++) {
		r = upek1001_reg_write(h, capture_1[i].addr, capture_1[i].val);
		if (r) {
			msg("capture_1 err %d at %d\n", r, i);
			break;
		}
	}
	msg("Reading bytes...\n");
	r = upek1001_read_bytes(h, buf, 512, &actual_len);
	msg("r = %d, actual: %d\n", r, actual_len);

	/* CAPTURE 2 */
	for (i = 0; i < ARRAY_SIZE(capture_2); i++) {
		r = upek1001_reg_write(h, capture_2[i].addr, capture_2[i].val);
		if (r) {
			msg("capture_2 err %d at %d\n", r, i);
			break;
		}
	}
	msg("Reading bytes...\n");
	r = upek1001_read_bytes(h, buf, 512, &actual_len);
	msg("r = %d, actual: %d\n", r, actual_len);

	/* CAPTURE 3 */
	for (i = 0; i < ARRAY_SIZE(capture_3); i++) {
		r = upek1001_reg_write(h, capture_3[i].addr, capture_3[i].val);
		if (r) {
			msg("capture_3 err %d at %d\n", r, i);
			break;
		}
	}
	msg("Reading bytes...\n");
	r = upek1001_read_bytes(h, buf, 512, &actual_len);
	msg("r = %d, actual: %d\n", r, actual_len);

	/* CAPTURE 4 */
	for (i = 0; i < ARRAY_SIZE(capture_4); i++) {
		r = upek1001_reg_write(h, capture_4[i].addr, capture_4[i].val);
		if (r) {
			msg("capture_4 err %d at %d\n", r, i);
			break;
		}
	}
	msg("Reading bytes...\n");
	r = upek1001_read_bytes(h, buf, 512, &actual_len);
	msg("r = %d, actual: %d\n", r, actual_len);

	/* CAPTURE 5 */
	for (i = 0; i < ARRAY_SIZE(capture_5); i++) {
		r = upek1001_reg_write(h, capture_5[i].addr, capture_5[i].val);
		if (r) {
			msg("capture_5 err %d at %d\n", r, i);
			break;
		}
	}

	/* CAPTURE image! */
	msg("Capturing image...\n");

	out = fopen("finger.bin", "wb");
	do {
		r = upek1001_read_bytes(h, buf, 4096, &actual_len);
		write_file(out, buf, 4096);
		//msg("r = %d, actual: %d\n", r, actual_len);
	} while (actual_len == 4096 && !aborted);

	fclose(out);
	msg("Deinit!...\n");
	/* DEINIT */
	for (i = 0; i < ARRAY_SIZE(deinit_1); i++) {
		r = upek1001_reg_write(h, deinit_1[i].addr, deinit_1[i].val);
		if (r) {
			msg("deinit_1 err %d at %d\n", r, i);
			break;
		}
	}

	msg("Probed device successfully!\n");

	return 0;
}

int main(int argc, char *argv[])
{
	int r;
	libusb_device_handle *h;
	libusb_context *ctx;

	struct sigaction sigact;

        sigact.sa_handler = sighandler;
        sigemptyset(&sigact.sa_mask);
        sigact.sa_flags = 0;
        sigaction(SIGINT, &sigact, NULL);
        sigaction(SIGTERM, &sigact, NULL);
        sigaction(SIGQUIT, &sigact, NULL);

	libusb_init(&ctx);

	h = libusb_open_device_with_vid_pid(ctx, 0x147e, 0x1001);
	if (!h) {
		msg_err("Can't open upek1001 device!\n");
		return 0;
	}
	
	r = libusb_claim_interface(h, 0);
	if (r < 0) {
		msg_err("Failed to claim interface 0\n");
		goto exit_closelibusb;
	}

	r = upek1001_test(h);
	if (r < 0) {
		msg_err("Failed to probe upek1001\n");
		goto exit_closelibusb;
	}

exit_closelibusb:
	libusb_close(h);
	libusb_exit(ctx);

	return 0;

}
