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



#define MAX_BUF_SIZE 1000000 // == MB

int write_sock(int sock, char *buff, size_t count) {
	int num_writ, curr_num_writ;
	
	num_writ = 0;
    while (num_writ < count) {
        curr_num_writ = send(sock, &buff[num_writ], count - num_writ, 0);
        if (curr_num_writ >= 0) {
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
	int num_read, curr_num_read;
	
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

int sendfile(int conn_sock, int file_fd) {
    char content[MAX_BUF_SIZE];
    int num_bytes_read;

    while ((num_bytes_read = read(file_fd, (char *)&content, MAX_BUF_SIZE)) != 0) {
    	if (num_bytes_read < 0) {
    		perror(strerror(errno));
    		return -1;
    	}
    	
        if (write_sock(conn_sock, (char *)&content, num_bytes_read) < 0) {
        	return -1;
        }
    }
    
    return 0;
}

void handle_connection(int conn_sock, int file_fd, uint64_t size_net) {
	uint64_t num_printable_host, num_printable_net;
	
	/* send N */
	if (write_sock(conn_sock, (char *)&size_net, sizeof(size_net)) < 0) {
		return;
	}
	
	/* send file */
	if (sendfile(conn_sock, file_fd) < 0) {
		return;
	}
	
	/* receive number of printable bytes */
	if (read_sock(conn_sock, (char *)&num_printable_net, sizeof(num_printable_net)) < 0) {
		return;
	}
	num_printable_host = be64toh(num_printable_net);
	
	printf("# of printable characters: %lu\n", num_printable_host);
}

int main(int argc, char *argv[]) {
	int port, file_fd, sock;
	uint64_t size_host, size_net;
	char *file_path, *ip;
	struct sockaddr_in serv_addr = {0};
	struct stat st;

	/* handle arguments */
	/* validate correct number of args */
    if (argc != 4) {
        perror("Incorrect number of arguments\n");
        exit(1);
    }
    
    ip = argv[1];
    port = atoi(argv[2]);
    file_path = argv[3];
    
    /* make sure file can be opened */
    file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
    	perror(strerror(errno));
    	exit(1);
    }
    
    
    /* create a TCP connection */
   	/* open a socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	/* connect to server */
	serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    	perror(strerror(errno));
    	exit(1);
    }
    
    
    /* handle connection */
    /* find N */
	stat(file_path, &st);
	size_host = (uint64_t)st.st_size;
	size_net = htobe64(size_host);
	
    handle_connection(sock, file_fd, size_net);
    
    
    /* close up */
    close(file_fd);
    close(sock);
    
}
