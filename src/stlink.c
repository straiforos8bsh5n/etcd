/* stlink/v2 device driver
   (c) Valentin Dudouyt, 2012 */

#include <stdio.h>
#include <libusb.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include "stlink.h"

stlink_context_t *stlink_init(unsigned int dev_id) {
	stlink_context_t *context = malloc(sizeof(stlink_context_t));
	assert(context);
	memset(context, 0, sizeof(stlink_context_t));

	libusb_device **devs;
	libusb_context *ctx = NULL;

	int r;
	ssize_t cnt;
	r = libusb_init(&ctx);
	if(r < 0) return(NULL);

	libusb_set_debug(ctx, 3);
	cnt = libusb_get_device_list(ctx, &devs);
	if(cnt < 0) return(NULL);

	context->dev_handle = libusb_open_device_with_vid_pid(ctx, 0x0483, 0x3748);
	context->ctx = ctx;
	assert(context->dev_handle);
	//r = libusb_claim_interface(context->dev_handle, 0);
	//assert(r >= 0);

	libusb_free_device_list(devs, 1); //free the list, unref the devices in it
	return context;
}

int msg_count;
void stlink_send_message(stlink_context_t *context, int count, ...) {
	va_list ap;
	char data[16];
	int i, r, actual;

	va_start(ap, count);
	memset(data, 0, 16);
	for(i = 0; i < count; i++)
		data[i] = va_arg(ap, int);
	r = libusb_bulk_transfer(context->dev_handle, (2 | LIBUSB_ENDPOINT_OUT), data, sizeof(data), &actual, 0);
	usleep(3000);
	if(context->debug)
		fprintf(stderr, "msg_count=%d r=%d actual=%d\n", msg_count, r, actual);
	assert(r == 0);
	msg_count++;
	return;
}

int stlink_read(stlink_context_t *context, char *buffer, int count) {
	int r, recv;
	r = libusb_bulk_transfer(context->dev_handle, (1 | LIBUSB_ENDPOINT_IN), buffer, count, &recv, 0);
	assert(r==0);
	if(context->debug && recv > 16) printf("Received block recv=%d\n", recv);
	return(recv);
}

int stlink_read1(stlink_context_t *context, int count) {
	char buf[16];
	int recv = stlink_read(context, buf, count);
	if(context->debug) {
		// Dumping received data
		int i;
		for(i = 0; i < recv; i++)
			fprintf(stderr, "%02x", buf[i]);
		fprintf(stderr, "\n");
	}
	return(recv);
}

int stlink_read_and_cmp(stlink_context_t *context, int count, ...) {
	va_list ap;
	char buf[16];
	int recv = stlink_read(context, buf, count);
	int i, ret = 0;
	va_start(ap, count);
	for(i = 0; i < count; i++) {
		if(buf[i] != va_arg(ap, int))
			ret++;
	}
	return(ret);
}

stlink_status_t stlink_swim_start(stlink_context_t *context) {
	msg_count = 0;
	int r, i;
	char buffer[16];
	stlink_send_message(context, 1, 0xf5);
	stlink_read1(context, 2);
	stlink_send_message(context, 2, 0xf3, 0x07);
	stlink_send_message(context, 1, 0xf4);
	stlink_send_message(context, 2, 0xf4, 0x0d);
	stlink_read1(context, 2);
	stlink_send_message(context, 3, 0xf4, 0x02, 0x01);
	stlink_read1(context, 8);
	for(i = 0; i < 2; i++) {
		stlink_send_message(context, 3, 0xf4, 0x07, 0x01);
		stlink_send_message(context, 3, 0xf4, 0x09, 0x01);
		stlink_read1(context, 4);
	}
	stlink_send_message(context, 3, 0xf4, 0x04, 0x01);
	stlink_send_message(context, 3, 0xf4, 0x09, 0x01);
	if(stlink_read_and_cmp(context, 4, 0x01, 0x00, 0x00, 0x00))
		return(STLK_SWIM_ERROR);
	return(STLK_OK);
}

int stlink_swim_read_range(stlink_context_t *context, char *buffer, unsigned int start, unsigned int length) {
	int recv = 0;
	int start_h = (start & 0xFF00) >> 8;
	int start_l = start & 0xFF;
stlink_send_message(context, 9, 0xf4, 0x07, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x08, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x07, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x04, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x03, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x05, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xa0);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xa0);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x08, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xa0);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xa0);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x99, 0xa0);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x99, 0xa0);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x99, 0xa0);
stlink_read1(context, 1);
stlink_send_message(context, 9, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x50, 0xcd, 0xa0);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0xcd, 0xa0);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x50, 0xcd, 0xa0);
stlink_read1(context, 1);
stlink_send_message(context, 9, 0xf4, 0x06, 0x00, 0x01, 0x00, 0x00, 0x50, 0xcd, 0xa0);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0xcd, 0xa0);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb0);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb0);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x03, 0x01, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb0);
stlink_send_message(context, 9, 0xf4, 0x09, 0x01, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb0);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb4);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb4);
stlink_read1(context, 4);
stlink_send_message(context, 8, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x50, 0xc6);
stlink_send_message(context, 8, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0xc6);
stlink_read1(context, 4);
stlink_send_message(context, 8, 0xf4, 0x0b, 0x00, 0x04, 0x00, 0x00, 0x4f, 0xfc);
stlink_send_message(context, 8, 0xf4, 0x09, 0x00, 0x04, 0x00, 0x00, 0x4f, 0xfc);
stlink_read1(context, 4);
stlink_send_message(context, 8, 0xf4, 0x0c, 0x00, 0x04, 0x00, 0x00, 0x4f, 0xfc);
stlink_read1(context, 4);
stlink_send_message(context, 8, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x50, 0x5f);
stlink_send_message(context, 8, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0x5f);
stlink_read1(context, 4);
stlink_send_message(context, 8, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x50, 0x5f);
stlink_read1(context, 1);
stlink_send_message(context, 7, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x48);
stlink_send_message(context, 7, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x48);
stlink_read1(context, 4);
stlink_send_message(context, 7, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x48);
stlink_read1(context, 1);
stlink_send_message(context, 8, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x48, 0x01);
stlink_send_message(context, 8, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x48, 0x01);
stlink_read1(context, 4);
stlink_send_message(context, 8, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x48, 0x01);
stlink_read1(context, 1);
stlink_send_message(context, 8, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x48, 0x02);
stlink_send_message(context, 8, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x48, 0x02);
stlink_read1(context, 4);
stlink_send_message(context, 8, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x48, 0x02);
stlink_read1(context, 1);
stlink_send_message(context, 8, 0xf4, 0x0b, 0x00, length, 0x00, 0x00, start_h, start_l);
stlink_send_message(context, 8, 0xf4, 0x09, 0x00, length, 0x00, 0x00, start_h, start_l);
stlink_read1(context, 4);
stlink_send_message(context, 8, 0xf4, 0x0c, 0x00, length, 0x00, 0x00, start_h, start_l);
// IN 64 bytes ommited
// IN 17 bytes ommited
	recv += stlink_read(context, buffer, length);
stlink_send_message(context, 8, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80);
stlink_send_message(context, 8, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80);
stlink_read1(context, 4);
stlink_send_message(context, 8, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80);
stlink_read1(context, 1);
stlink_send_message(context, 9, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x05, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x07, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_read1(context, 4);
stlink_send_message(context, 9, 0xf4, 0x03, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_send_message(context, 9, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6);
stlink_read1(context, 4);
	return(recv);
}

int stlink_swim_write_range(stlink_context_t *context, char *buffer, unsigned int start, unsigned int length) {
stlink_send_message(context, 10, 0xf4, 0x07, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_read1(context, 4); // 00000000
stlink_send_message(context, 10, 0xf4, 0x08, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_read1(context, 4); // 00000000
stlink_send_message(context, 10, 0xf4, 0x07, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_read1(context, 4); // 00000000
stlink_send_message(context, 10, 0xf4, 0x04, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_read1(context, 4); // 01000000
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_read1(context, 4); // 01000000
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_read1(context, 4); // 00000000
stlink_send_message(context, 10, 0xf4, 0x03, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_read1(context, 4); // 00000000
stlink_send_message(context, 10, 0xf4, 0x05, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb6, 0xff);
stlink_read1(context, 4); // 00000000
stlink_send_message(context, 10, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xa0, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xa0, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x08, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xa0, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xa0, 0xff);
stlink_read1(context, 4); // 00000000
stlink_send_message(context, 10, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x99, 0xa0, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x99, 0xa0, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x99, 0xa0, 0xff);
stlink_read1(context, 1); // 08
stlink_send_message(context, 10, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x50, 0xcd, 0xa0, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0xcd, 0xa0, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x50, 0xcd, 0xa0, 0xff);
stlink_read1(context, 1); // 00
stlink_send_message(context, 10, 0xf4, 0x06, 0x00, 0x01, 0x00, 0x00, 0x50, 0xcd, 0xa0, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0xcd, 0xa0, 0xff);
stlink_read1(context, 4); // 00000000
stlink_send_message(context, 10, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb0, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb0, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x03, 0x01, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb0, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x01, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb0, 0xff);
stlink_read1(context, 4); // 00000000
stlink_send_message(context, 10, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb4, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x80, 0xb4, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x50, 0xc6, 0x00, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0xc6, 0x00, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x0b, 0x00, 0x04, 0x00, 0x00, 0x4f, 0xfc, 0x00, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x04, 0x00, 0x00, 0x4f, 0xfc, 0x00, 0xff);
stlink_read1(context, 4); // 00040000
stlink_send_message(context, 10, 0xf4, 0x0c, 0x00, 0x04, 0x00, 0x00, 0x4f, 0xfc, 0x00, 0xff);
stlink_read1(context, 4); // 67671003
stlink_send_message(context, 10, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x50, 0x5f, 0x00, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0x5f, 0x00, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x50, 0x5f, 0x00, 0xff);
stlink_read1(context, 1); // 40
stlink_send_message(context, 10, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x48, 0x00, 0x00, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x48, 0x00, 0x00, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x48, 0x00, 0x00, 0xff);
stlink_read1(context, 1); // 00
stlink_send_message(context, 10, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x48, 0x01, 0x00, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x48, 0x01, 0x00, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x48, 0x01, 0x00, 0xff);
stlink_read1(context, 1); // 00
stlink_send_message(context, 10, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x48, 0x02, 0x00, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x48, 0x02, 0x00, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x48, 0x02, 0x00, 0xff);
stlink_read1(context, 1); // ff
stlink_send_message(context, 10, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x50, 0x62, 0x56, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0x62, 0x56, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x50, 0x62, 0xae, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0x62, 0xae, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x50, 0x64, 0xae, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0x64, 0xae, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x50, 0x64, 0x56, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0x64, 0x56, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x50, 0x5f, 0x56, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0x5f, 0x56, 0xff);
stlink_read1(context, 4); // 00010000
stlink_send_message(context, 10, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x50, 0x5f, 0x56, 0xff);
stlink_read1(context, 1); // 4a
stlink_send_message(context, 10, 0xf4, 0x0a, 0x00, 0x02, 0x00, 0x00, 0x50, 0x5b, 0x00, 0xff);
stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x02, 0x00, 0x00, 0x50, 0x5b, 0x00, 0xff);
stlink_read1(context, 4); // 00020000
	int i, sent = 0;
	for(i = 0; i < length; i++) {
		int addr_h = ((start + i) & 0xFF00) >> 8;
		int addr_l = (start + i) & 0xFF;
		stlink_send_message(context, 10, 0xf4, 0x0a, 0x00, 0x01, 0x00, 0x00, addr_h, addr_l, buffer[i], 0xff);
		stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, addr_h, addr_l, buffer[i], 0xff);
		stlink_read1(context, 4); // 00010000
		stlink_send_message(context, 10, 0xf4, 0x0b, 0x00, 0x01, 0x00, 0x00, 0x50, 0x5f, buffer[i], 0xff);
		stlink_send_message(context, 10, 0xf4, 0x09, 0x00, 0x01, 0x00, 0x00, 0x50, 0x5f, buffer[i], 0xff);
		stlink_read1(context, 4); // 00010000
		stlink_send_message(context, 10, 0xf4, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x50, 0x5f, buffer[i], 0xff);
		//stlink_read1(context, 1); // 4e
		if(!stlink_read_and_cmp(context, 1, 0x4e))
			sent++;
	}
	return(sent);
}

void stlink_exit(stlink_context_t *context) {
	libusb_exit(context->ctx); //close the session
}
