#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEST "/data/homebrew/"

static char g_url[2048], g_name[256], g_msg[256] = "idle";
static volatile long long g_total, g_done;
static volatile int g_state; /* 0 idle, 1 downloading, 2 done, 3 error */

static void fail(const char *m) {
  snprintf(g_msg, sizeof g_msg, "%s", m);
  g_state = 3;
}

static void url_decode(char *s, int plus) {
  char *o = s;
  while (*s) {
    if (*s == '%' && isxdigit((unsigned char)s[1]) &&
        isxdigit((unsigned char)s[2])) {
      char h[3] = {s[1], s[2], 0};
      *o++ = (char)strtol(h, NULL, 16);
      s += 3;
    } else if (plus && *s == '+') {
      *o++ = ' ';
      s++;
    } else {
      *o++ = *s++;
    }
  }
  *o = 0;
}

static void send_all(int c, const char *p, size_t n) {
  while (n) {
    ssize_t w = write(c, p, n);
    if (w <= 0) return;
    p += w;
    n -= (size_t)w;
  }
}

static void reply(int c, const char *type, const char *body) {
  char h[256];
  int n = snprintf(h, sizeof h,
                   "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n"
                   "Cache-Control: no-store\r\nConnection: close\r\n\r\n",
                   type);
  send_all(c, h, (size_t)n);
  send_all(c, body, strlen(body));
}

static void *download(void *arg) {
  (void)arg;
  char host[256], path[1536], port[8] = "80";
  const char *u = g_url;

  if (strncmp(u, "http://", 7) != 0) {
    fail("only http:// links work for now");
    return NULL;
  }
  u += 7;
  const char *slash = strchr(u, '/');
  size_t hl = slash ? (size_t)(slash - u) : strlen(u);
  if (hl == 0 || hl >= sizeof host) {
    fail("bad host");
    return NULL;
  }
  memcpy(host, u, hl);
  host[hl] = 0;
  snprintf(path, sizeof path, "%s", slash ? slash : "/");
  char *colon = strchr(host, ':');
  if (colon) {
    *colon = 0;
    snprintf(port, sizeof port, "%s", colon + 1);
  }

  char fname[256];
  {
    char tmp[1536];
    snprintf(tmp, sizeof tmp, "%s", path);
    char *q = strchr(tmp, '?');
    if (q) *q = 0;
    char *b = strrchr(tmp, '/');
    b = b ? b + 1 : tmp;
    url_decode(b, 0);
    size_t j = 0;
    for (size_t i = 0; b[i] && j < sizeof fname - 1; i++)
      fname[j++] = (b[i] == '/' || b[i] == '\\') ? '_' : b[i];
    fname[j] = 0;
    if (!fname[0] || !strcmp(fname, ".") || !strcmp(fname, ".."))
      snprintf(fname, sizeof fname, "download.bin");
  }
  snprintf(g_name, sizeof g_name, "%s", fname);

  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(host, port, &hints, &res) != 0) {
    fail("DNS lookup failed");
    return NULL;
  }
  int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (s < 0 || connect(s, res->ai_addr, res->ai_addrlen) < 0) {
    freeaddrinfo(res);
    if (s >= 0) close(s);
    fail("connect failed");
    return NULL;
  }
  freeaddrinfo(res);

  char req[2048];
  int n = snprintf(req, sizeof req,
                   "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: Ps5Downloadly\r\n"
                   "Accept: */*\r\nConnection: close\r\n\r\n",
                   path, host);
  send_all(s, req, (size_t)n);

  static char hdr[8192];
  size_t have = 0;
  char *end = NULL;
  while (!end && have < sizeof hdr - 1) {
    ssize_t r = recv(s, hdr + have, sizeof hdr - 1 - have, 0);
    if (r <= 0) break;
    have += (size_t)r;
    hdr[have] = 0;
    end = strstr(hdr, "\r\n\r\n");
  }
  if (!end) {
    close(s);
    fail("bad response from server");
    return NULL;
  }
  int code = atoi(hdr + 9);
  if (code != 200) {
    char m[96];
    snprintf(m, sizeof m, "server returned HTTP %d", code);
    close(s);
    fail(m);
    return NULL;
  }
  g_total = 0;
  char *cl = strcasestr(hdr, "\r\nContent-Length:");
  if (cl) g_total = atoll(cl + 17);

  char full[512];
  snprintf(full, sizeof full, DEST "%s", fname);
  int fd = open(full, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) {
    close(s);
    fail("cannot create file in /data/homebrew");
    return NULL;
  }

  char *body = end + 4;
  size_t blen = have - (size_t)(body - hdr);
  if (blen) {
    if (write(fd, body, blen) != (ssize_t)blen) {
      close(fd);
      close(s);
      fail("write error");
      return NULL;
    }
    g_done += (long long)blen;
  }

  size_t bufsz = 256 * 1024;
  char *buf = malloc(bufsz);
  if (!buf) {
    close(fd);
    close(s);
    fail("out of memory");
    return NULL;
  }
  for (;;) {
    ssize_t r = recv(s, buf, bufsz, 0);
    if (r <= 0) break;
    if (write(fd, buf, (size_t)r) != r) {
      free(buf);
      close(fd);
      close(s);
      fail("write error (disk full?)");
      return NULL;
    }
    g_done += r;
  }
  free(buf);
  close(fd);
  close(s);
  if (g_total && g_done != g_total) {
    fail("download incomplete");
    return NULL;
  }
  snprintf(g_msg, sizeof g_msg, "saved to %s", full);
  g_state = 2;
  return NULL;
}

static const char *PAGE =
    "<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'>"
    "<title>Ps5Downloadly</title>"
    "<body style='font-family:sans-serif;margin:16px'><h2>Ps5Downloadly</h2>"
    "<input id=u placeholder='http link' style='width:100%;font-size:16px;padding:8px'>"
    "<button onclick='go()' style='font-size:18px;padding:10px;margin-top:8px'>Start download</button>"
    "<pre id=s></pre>"
    "<script>"
    "function tick(){fetch('/status').then(function(r){return r.text()}).then(function(t){document.getElementById('s').textContent=t})}"
    "function go(){fetch('/start?u='+encodeURIComponent(document.getElementById('u').value)).then(tick)}"
    "setInterval(tick,1000);tick();"
    "</script>";

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
  if (listen(srv, 8) < 0) return 1;

  for (;;) {
    int c = accept(srv, NULL, NULL);
    if (c < 0) continue;
    char buf[4096];
    ssize_t r = read(c, buf, sizeof(buf) - 1);
    if (r <= 0) {
      close(c);
      continue;
    }
    buf[r] = 0;

    if (strncmp(buf, "GET /start?u=", 13) == 0) {
      if (g_state == 1) {
        reply(c, "text/plain", "busy");
      } else {
        char *v = buf + 13;
        char *e = v;
        while (*e && *e != ' ' && *e != '&' && *e != '\r') e++;
        *e = 0;
        url_decode(v, 1);
        snprintf(g_url, sizeof g_url, "%s", v);
        g_state = 1;
        g_done = 0;
        g_total = 0;
        g_name[0] = 0;
        snprintf(g_msg, sizeof g_msg, "starting");
        pthread_t t;
        pthread_attr_t at;
        pthread_attr_init(&at);
        pthread_attr_setdetachstate(&at, PTHREAD_CREATE_DETACHED);
        if (pthread_create(&t, &at, download, NULL) != 0) fail("thread error");
        reply(c, "text/plain", "ok");
      }
    } else if (strncmp(buf, "GET /status", 11) == 0) {
      static const char *names[] = {"idle", "downloading", "done", "error"};
      char out[768];
      double d = (double)g_done / 1048576.0;
      double t = (double)g_total / 1048576.0;
      int pct = g_total ? (int)(g_done * 100 / g_total) : 0;
      snprintf(out, sizeof out,
               "state: %s\nfile: %s\n%.1f / %.1f MB (%d%%)\n%s\n",
               names[g_state], g_name, d, t, pct, g_msg);
      reply(c, "text/plain", out);
    } else {
      reply(c, "text/html", PAGE);
    }
    close(c);
  }
}
