#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>


#define START_VALUE 32
#define END_VALUE 126
#define NUM_PRINT_CHARS 95
#define MAX_BUF_SIZE 1000000

int digest_data(int *statistics, int size, int sock) {
	int new_statistics[NUM_PRINT_CHARS] = {0};
	int recv_buf[MAX_BUF_SIZE];
	int num_bytes_recv, i, ind, num_print = 0;
	
	while (size > 0) {
		num_bytes_recv = recv(sock, &recv_buf, MAX_BUF_SIZE, 0);
		
		/* update new statistics */
		for (i = 0; i < num_bytes_recv; ++i) {
			ind = recv_buf[i];
			if (ind >= 32 && ind <= 126) { /* is printable! */
				ind -= START_VALUE;
				new_statistics[ind]++;
				num_print++;
			}
		}
		
		size -= num_bytes_recv;
	}
	
	/* update statistics */
	for (i = 0; i < NUM_PRINT_CHARS; ++i) {
		statistics[i] += new_statistics[i];
	}
	
	return num_print;
}

int main(int argc, char *argv[]) {
	int port, sock_fd, acc_sock_fd, size_recv, num_print;
	struct sockaddr_in serv_addr;
	int *statistics = (int *)calloc(MAX_BUF_SIZE, sizeof(int)); /* printable chars range from 32 to 126 */
	char *recv_buf;
	
	if (argc != 2) {
		printf("Incorrect number of arguments\n");
		exit(1);
	}
	
	/* handle argument */
	port = atoi(argv[1]);
	
	/* listen to incoming TCP connections */
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(struct sockaddr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr));
	
	listen(sock_fd, 10);
	printf("listening on port %d\n", port);
	
	while (1) {
		/* accept connection */
		acc_sock_fd = accept(sock_fd, NULL, NULL);
		printf("accepted_connection\n");
		
		/* receive size of stream */
		recv(acc_sock_fd, &recv_buf, 4, 0); 
		size_recv = ntohl(*recv_buf);
		printf("received size of message\n");
		
		/* receive actual data and update statistics accordingly */
		num_print = digest_data(statistics, size_recv, acc_sock_fd);
		printf("received data\n");
		
		/* send a response to the client */
		send(acc_sock_fd, (char *)&num_print, 4, 0);
		printf("sent response\n");
	}
}
