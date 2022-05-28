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

#define MAX_BUF_SIZE 1000000 // == MB
#define NUM_PRINT_CHARS 95


int write_sock(int sock, char *buff, size_t count) {
	int num_writ, curr_num_writ;
	
	num_writ = 0;
	
    while (num_writ < count) {
        curr_num_writ = send(sock, &buff[num_writ], count - num_writ, 0);
        if (curr_num_writ > 0) {
            num_writ += curr_num_writ;
        } 
        else {
        	perror(strerror(errno));
            return -1;
        }
    }
    
    return 0;
}

int read_sock(int sock, char *buff, size_t count) {
	int num_read, curr_num_read;
	
	num_read = 0;
	printf("count = %d\n", (int)count);
    while (num_read < count) {
    	printf("num_read = %d\n", num_read);
        curr_num_read = recv(sock, &buff[num_read], count - num_read, 0);
        printf("curr_num_read = %d\n", curr_num_read);
        if (curr_num_read >= 0) {
            num_read += curr_num_read;
        } 
        else {
        	perror(strerror(errno));
            return -1;
        }
    }
    
    return num_read;
}

int update_statistics(int sock, int *stats, int size) {
	int new_stats[NUM_PRINT_CHARS] = {0};
    char buff[MAX_BUF_SIZE];
    int num_bytes_recv, i, ind, num_printable = 0;

    while (size > 0) {
        num_bytes_recv = read_sock(sock, (char *)&buff, MAX_BUF_SIZE);
        if (num_bytes_recv < 0) {
        	return -1;
        }

        /* update new statistics */
        for (i = 0; i < num_bytes_recv; ++i) {
            ind = buff[i];
            if (ind >= 32 && ind <= 126) { /* is printable! */
                ind -= 32;
                new_stats[ind]++;
                num_printable++;
            }
        }

        size -= num_bytes_recv;
    }

    /* update statistics */
    for (i = 0; i < NUM_PRINT_CHARS; ++i) {
        stats[i] += new_stats[i];
    }

    return num_printable;
}


void handle_connection(int conn_sock, int *stats) {
	uint32_t size, num_printable;
	
	/* receive size */
	printf("receiving N...\n");
	if (read_sock(conn_sock, (char *)&size, sizeof(size)) < 0) {
		return;
	}
	
	/* update statistics */
	printf("updating statistics...");
	num_printable = update_statistics(conn_sock, stats, size);
	if (num_printable < 0) {
		return;
	}
	
	/* send number of printable chars */
	printf("sending number of printable chars\n");
	if (write_sock(conn_sock, (char *)&num_printable, sizeof(num_printable)) < 0) {
		return;
	}
}


int main(int argc, char *argv[]) {
	int port, lis_sock, conn_sock;
	struct sockaddr_in serv_addr = {0};
	int *stats = (int *)calloc(MAX_BUF_SIZE, sizeof(int)); /* printable chars range from 32 to 126 */
	
	/* handle arguments */
	if (argc != 2) {
        perror("Incorrect number of arguments\n");
        exit(1);
    }
    
	port = atoi(argv[1]);
	
	
	/* listen to incoming TCP connections */
	/* open a socket */
	lis_sock = socket(AF_INET, SOCK_STREAM, 0);
	
	/* bind the socket */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(lis_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0) {
    	perror(strerror(errno));
    	exit(1);
    }
    
    /* listen on port */
    printf("listening on port %d...\n", port);
    if (listen(lis_sock, 10) != 0) {
    	perror(strerror(errno));
    	exit(1);
    }
    
    
    /* accept and handle connection */
    while (1) {
    	/* accept connection */
    	printf("accepting connection...\n");
    	conn_sock = accept(lis_sock, NULL, NULL);
    	if (conn_sock < 0) {
    		perror(strerror(errno));
    		exit(1);
    	}
    	
    	/* handle connection */
    	handle_connection(conn_sock, stats);
    	
    	/* close up */
    	close(conn_sock);
    }
    	
}
