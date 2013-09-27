#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define TIMER_INT       60

int sock;
void ahandler();
int error (int num);
int main(int argc, char **argv) {
  struct protoent *pr;
  struct sockaddr_in sa;
  struct in_addr inp;
  int one = 1;
  int x = 1;
  int ret;
  char buf[4192];
  char *ptr;
  char ip[16];
  struct hostent *host;
  char hostname[1024];

  char agent[] = "Mozilla/4.0 (compatible; MSIE 5.5; Windows NT 5.0)";
  char *send="GET /%s HTTP/1.0\r\nUser-Agent: %s\r\nAccept: */*\r\nConnection: Close\r\nHost: %s\r\n\r\n";

  signal(SIGALRM, ahandler);
  alarm(TIMER_INT);

  if (argc < 2) {
    fprintf(stderr, "usage: %s domain_name\nE.g. %s www.yahoo.com\n", argv[0], argv[0]);
    return(0);
  }

  sprintf(hostname, "%s", argv[1]);
  if ((ptr = strstr(hostname, "/"))!=NULL) {
     *ptr = '\0';
     sprintf(buf, send, ptr+1, agent, hostname);
  } else {
     sprintf(buf, send, "", agent, argv[1]);
  }

  host = gethostbyname(hostname);
  if (host != NULL) {
    memcpy(&inp, host->h_addr_list[0], host->h_length);
    sprintf(ip, "%s", inet_ntoa(inp));
  } else
      error(0);

  pr = getprotobyname("tcp");
  if ((sock = socket(PF_INET, SOCK_STREAM, pr->p_proto)) == -1)
    error(1);
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &one, sizeof(one)) == -1)
    error(2);

  if (inet_aton(ip, &inp) == 0)
    error(3);
  sa.sin_port = htons(80);
  sa.sin_addr = inp;
  sa.sin_family = AF_INET;
  if (connect(sock, (struct sockaddr *)&sa, sizeof(sa)) == -1)
    error(4);
  write(sock, buf, strlen(buf));

  while ((ret = read(sock, buf, 4192)) != 0) {
    buf[ret]='\0';
    /*if (x == 1) {
      if ((ptr = strstr(buf, "\r\n\r\n")) != NULL)
        strcpy(buf,ptr+4);
    }*/
    fwrite(buf, ret, sizeof(char), stdout);
    x++;
  }

  if (close(sock) == -1)
    error(99);

  return(0);
}

int error(int num) {
  switch (num) {
  case 0:
    fprintf(stderr, "Could not resolve host name!\n");
    break;
  case 1:
    fprintf(stderr, "Could not open the socket!\n");
    break;
  case 2:
    fprintf(stderr, "Could not set sock options!\n");
    break;
  case 3:
    fprintf(stderr, "Could not set inet_aton options!\n");
    break;
  case 4:
    fprintf(stderr, "Could not connect Socket!\n");
    break;
  case 99:
    fprintf(stderr, "Could not close the socket!\n");
    break;
  }
  exit(num);
}

void ahandler() {
  printf("Timed out.\n");

  if (sock)
   close(sock);

  exit(1);
}