#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>

#define SERVICE_NAME "5099"
#define HOST_NAME "localhost"

int connect_server(int argc, char *argv[])
{
	int sd;
	int res;
	struct addrinfo hints;
	struct addrinfo *addr_i;
	struct addrinfo *hostaddr = 0;

	char *host = HOST_NAME;
	char __tmp[] = SERVICE_NAME;
	char *service = __tmp;

	memset(&hints, 0, sizeof (hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICHOST|AI_NUMERICSERV;

	if (argc > 1) {
		if (strcmp(argv[1], "-") != 0) {
			host = argv[1];
		}
		if (argc > 2) {
			service = argv[2];
		}
	}

	printf("resolving %s/%s ...", host, service);
	fflush(stdout);

	res = getaddrinfo(host, service, &hints, &hostaddr);
	if (res == EAI_NONAME) {
		hints.ai_flags = AI_CANONNAME|AI_NUMERICSERV;
		res = getaddrinfo(host, service, &hints, &hostaddr);
	}

	if (res != 0 || hostaddr == 0) {
		printf("failed\n");
		exit(1);
	}

	printf("success\n");

	for (addr_i = hostaddr; addr_i != 0; addr_i = addr_i->ai_next) {
		char name[1024];
		char serv_name[64];
		if ((sd = socket(addr_i->ai_family, addr_i->ai_socktype, 0)) == -1) {
			continue;
		}

		getnameinfo(addr_i->ai_addr, addr_i->ai_addrlen,
			name, sizeof(name), serv_name, sizeof(serv_name),
			NI_NUMERICHOST | NI_NUMERICSERV);
		printf("connecting to %s/%s ... ", name, serv_name);
		fflush(stdout);

		if (connect(sd, addr_i->ai_addr, addr_i->ai_addrlen) == 0) {
			printf("success\n");
			break;
		}

		perror("");
		close(sd);
	}

	freeaddrinfo(hostaddr);
	return addr_i == 0 ? -1 : sd;
}

int main(int argc, char *argv[])
{
	int i;
	int len;
	char buf[1024];
	int sd = connect_server(argc, argv);
	if (sd == -1) {
		exit(1);
	}

	for (i = 0; i < 1; ++i) {
		struct timespec ts[2];
		clock_gettime(CLOCK_MONOTONIC, &ts[0]);
		len = send(sd, buf, 128, 0);
		//int recv_len = recv(sd, buf, sizeof (buf), 0);
		clock_gettime(CLOCK_MONOTONIC, &ts[1]);
		long nsec = ts[1].tv_nsec - ts[0].tv_nsec;
		//printf("sent=%d, recv=%d, time=%ld.%03ldus\n", len, recv_len, nsec / 1000, nsec % 1000);
	}

	close(sd);
	return 0;
}


