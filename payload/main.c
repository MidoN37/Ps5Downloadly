#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
  int srv = socket(AF_INET, SOCK_STREAM, 0);
  int one = 1;
  setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

  struct sockaddr_in a;
  memset(&a, 0, sizeof(a));
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = htonl(INADDR_ANY);
  a.sin_port = htons(8085);

  if (bind(srv, (struct sockaddr *)&a, sizeof(a)) < 0) return 1;
  if (listen(srv, 5) < 0) return 1;

  const char *resp =
      "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n"
      "Connection: close\r\n\r\nPs5Downloadly is alive\n";

  for (;;) {
    int c = accept(srv, NULL, NULL);
    if (c < 0) continue;
    char buf[1024];
    read(c, buf, sizeof(buf));
    write(c, resp, strlen(resp));
    close(c);
  }
}
