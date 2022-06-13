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
#include <endian.h>
#include <stdint.h>
#include <signal.h>
#include <stdatomic.h>

#define MAX_BUF_SIZE 1000000 // == MB
#define MIN_PRINT_CHAR 32
#define MAX_PRINT_CHAR 126
#define NUM_PRINT_CHARS 95

atomic_int received_sigint;
uint64_t *stats;

void print_stats() {
	int i;
	char c;
	
	for (i = 0; i < NUM_PRINT_CHARS; ++i) {
		c = (char)(i + MIN_PRINT_CHAR);
		printf("char '%c' : %lu times\n", c, stats[i]);
	}
}

void handle_sigint_in_handle_connection() {
	received_sigint = 1;
}

void handle_sigint_in_main() {
	print_stats();
	exit(0);
}

int write_sock(int sock, char *buff, size_t count) {
	int num_writ, curr_num_writ;
	
	num_writ = 0;
	
    while (num_writ < count) {
        curr_num_writ = send(sock, &buff[num_writ], count - num_writ, 0);
        if (curr_num_writ > 0) {
            num_writ += curr_num_writ;
        } 
        else if (curr_num_writ == 0) {
        	return num_writ;
        }
        else {
        	perror(strerror(errno));
            return -1;
        }
    }
    
    return 0;
}

int read_sock(int sock, char *buff, size_t count) {
	long num_read, curr_num_read;
	
	num_read = 0;
    while (num_read < count) {
        curr_num_read = recv(sock, &buff[num_read], count - num_read, 0);
        if (curr_num_read > 0) {
            num_read += curr_num_read;
        } 
        else if (curr_num_read == 0) {
        	return num_read;
        }
        else {
        	perror(strerror(errno));
            return -1;
        }
    }
    return num_read;
}

int update_statistics(int sock, uint64_t size_host) {
	uint64_t new_stats[NUM_PRINT_CHARS] = {0};
    char buff[MAX_BUF_SIZE];
    int curr_size_host, num_bytes_recv, i, ind, num_printable = 0;

	while (size_host > 0) {
		/* find size to pass to read_sock */
		if (size_host > MAX_BUF_SIZE) {
			curr_size_host = MAX_BUF_SIZE;
		}
		else {
			curr_size_host = size_host;
		}
		
		/* read socket */
    	num_bytes_recv = read_sock(sock, (char *)&buff, curr_size_host);
    	if (num_bytes_recv < 0) {
    		return -1;
    	}

    	/* update new statistics */
    	for (i = 0; i < num_bytes_recv; ++i) {
        	ind = buff[i];
        	if (ind >= MIN_PRINT_CHAR && ind <= MAX_PRINT_CHAR) { /* is printable! */
            	ind -= MIN_PRINT_CHAR;
             	new_stats[ind]++;
           		num_printable++;
        	}
		}

		size_host -= num_bytes_recv;
	}

    /* update statistics */
    for (i = 0; i < NUM_PRINT_CHARS; ++i) {
        stats[i] += new_stats[i];
    }

    return num_printable;
}


void handle_connection(int conn_sock) {
	uint64_t size_net, size_host, num_printable_host, num_printable_net;
	
	/* handle SIGINT */
	struct sigaction act = {.sa_handler = handle_sigint_in_handle_connection};
	
	if (sigaction(SIGINT, &act, NULL) == -1) {
		perror(strerror(errno));
		exit(1);
	}
	
	/* receive size */
	if (read_sock(conn_sock, (char *)&size_net, sizeof(size_net)) < 0) {
		return;
	}
	size_host = be64toh(size_net);
	
	/* update statistics */
	num_printable_host = update_statistics(conn_sock, size_host);
	if (num_printable_host < 0) {
		return;
	}
	num_printable_net = htobe64(num_printable_host);
	
	/* send number of printable chars */
	if (write_sock(conn_sock, (char *)&num_printable_net, sizeof(num_printable_net)) < 0) {
		return;
	}
}


int main(int argc, char *argv[]) {
	int port, lis_sock, conn_sock;
	struct sockaddr_in serv_addr = {0};
	
	/* handle SIGINT */
	struct sigaction act = {.sa_handler = handle_sigint_in_main};
	
	if (sigaction(SIGINT, &act, NULL) == -1) {
		perror(strerror(errno));
		exit(1);
	}
	
	/* initialize global variables */
	received_sigint = 0;
	stats = (uint64_t *)calloc(MAX_BUF_SIZE, sizeof(uint64_t)); /* printable chars range from 32 to 126 */
	
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
    if (listen(lis_sock, 10) != 0) {
    	perror(strerror(errno));
    	exit(1);
    }
    
    
    /* accept and handle connection */
    while (1) {
    	/* handle sigint */
    	if (sigaction(SIGINT, &act, NULL) == -1) {
			perror(strerror(errno));
			exit(1);
		}
	
    	/* accept connection */
    	conn_sock = accept(lis_sock, NULL, NULL);
    	if (conn_sock < 0) {
    		perror(strerror(errno));
    		exit(1);
    	}
    	
    	/* handle connection */
    	handle_connection(conn_sock);
    	
    	if (received_sigint) {
    		handle_sigint_in_main();
    	}
    	
    	/* close up */
    	close(conn_sock);
    }
    	
}
