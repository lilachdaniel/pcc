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

void sendfile(int sock_fd, int file_fd) {
	char content[MAX_BUF_SIZE];
	int num_bytes_read;
	
	while ((num_bytes_read = read(file_fd, &content, MAX_BUF_SIZE)) != 0) {
		send(sock_fd, content, num_bytes_read, 0);
	}
}

int main(int argc, char *argv[]) {
	int port, file_fd, sock_fd, con_sock_fd, file_size;
	struct sockaddr_in serv_addr;
	struct stat st;
	char *recv_buf, *file_path, *ip;
	
	/* validate correct number of args */
	if (argc != 4) {
		printf("Incorrect number of arguments\n");
		exit(1);
	}
	
	/* handle arguments */
	ip = argv[1]; // handle error
	port = atoi(argv[2]);
	file_path = argv[3];
	file_fd = open(file_path, O_RDONLY); // handle error
	
	/* create a TCP connection */
	sock_fd = socket(AF_INET, SOCK_STREAM, 0); // handle error
	
	memset(&serv_addr, 0, sizeof(struct sockaddr)); // handle error
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	
	con_sock_fd = connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr)); // handle error
	printf("connected to server\n");
	
	/* send N */
	stat(file_path, &st); // handle error
	file_size = st.st_size;
	send(con_sock_fd, (char *)&file_size, 4, 0); // handle error
	printf("sent size of message\n");
	
	/* transfer the contents of the file to the server */
	sendfile(file_fd, con_sock_fd); // handle error
	printf("sent message\n");
	
	/* receive answer */
	recv(con_sock_fd, &recv_buf, 4, 0); // handle error
	printf("# of printable characters: %u\n", ntohl(*recv_buf));
	
	/* close up */
	close(file_fd);
	close(sock_fd);
	close(con_sock_fd);
}
