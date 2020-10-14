#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define SERVER "192.168.11.100"
#define BUFLEN 512
#define PORT 2137

void die(char *s)
{
    perror(s);
    exit(1);
}

int main(void) {
    struct sockaddr_in client;
    int s, slen = sizeof(client);
    char buf[BUFLEN];
    char message[BUFLEN];

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket");
    }

    memset((char *) &client, 0, sizeof(client)); 
    client.sin_family = AF_INET;
    client.sin_port = htons(PORT);

    if (inet_aton(SERVER , &client.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    int fifo = open("/tmp/bassfifo", O_CREAT);
    if (fifo < 0) {
        die("fifo");
    }

    for (int i = 0; i < 10000; i++) {
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);

        for (int j = 0; j < 44100 * 4 / 512 + 1; j++) {
            ssize_t read_bytes = 0;
            do {
                read_bytes += read(fifo, &message[read_bytes], BUFLEN - read_bytes);
            } while (read_bytes < BUFLEN);

            for (int k = 0; k < BUFLEN; k++) {
                putchar(message[k]);
            }

            if (sendto(s, message, BUFLEN, 0 , (struct sockaddr *) &client, slen)==-1)
            {
                die("sendto()");
            }

            while (1) {
                clock_gettime(CLOCK_MONOTONIC_RAW, &end);
                uint64_t delta_ns = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
                if (delta_ns > j * (1000000000 / (44100 * 4 / 512))) {
                    break;
                }
            }
        }
        /* clock_gettime(CLOCK_MONOTONIC_RAW, &end); */
        /* uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000; */
    }
}
