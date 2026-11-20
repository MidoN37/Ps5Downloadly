#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int sceNetInit(void);
int sceNetPoolCreate(const char *, int, int);
int sceSslInit(size_t);
int sceHttp2Init(int, int, size_t, int);
int sceHttp2CreateTemplate(int, const char *, int, int);
int sceHttp2DeleteTemplate(int);
int sceHttp2CreateRequestWithURL(int, const char *, const char *, uint64_t);
int sceHttp2DeleteRequest(int);
int sceHttp2SendRequest(int, const void *, size_t);
int sceHttp2GetStatusCode(int, int *);
int sceHttp2ReadData(int, void *, size_t);
int sceHttp2SetAutoRedirect(int, int);
int sceHttp2SetConnectTimeOut(int, uint32_t);
int sceHttp2GetResponseContentLength(int, int *, uint64_t *);
int sceHttp2SetSslCallback(int, int (*)(int, unsigned int, void *const[], int, void *), void *);

static volatile unsigned int g_ssl_err = 0xffffffffu;

static int ssl_cb(int ssl_id, unsigned int verify_err, void *const certs[],
                  int cert_num, void *arg) {
  (void)ssl_id; (void)certs; (void)cert_num; (void)arg;
  g_ssl_err = verify_err;
  return 0; /* accept certificate */
}

#define DEST "/data/homebrew/"
#define PORT 8086
#define BUILD "r3 ssl-callback, replaces old copy"

static char g_url[2048], g_name[256], g_msg[256] = "idle";
static volatile long long g_total, g_done;
static volatile int g_state; /* 0 idle, 1 downloading, 2 done, 3 error */
static int g_ctx = -1;

static void fail(const char *m) {
  snprintf(g_msg, sizeof g_msg, "%s", m);
  g_state = 3;
}

static void failc(const char *what, int code) {
  char m[160];
  snprintf(m, sizeof m, "%s failed (0x%x)", what, (unsigned)code);
  fail(m);
}

static int net_init(void) {
  if (g_ctx >= 0) return 0;
  if (sceNetInit() < 0) return -1;
  int pool = sceNetPoolCreate("ps5dl", 64 * 1024, 0);
  if (pool < 0) return -2;
  int ssl = sceSslInit(512 * 1024);
  if (ssl < 0) return -3;
  int ctx = sceHttp2Init(pool, ssl, 512 * 1024, 1);
  if (ctx < 0) return -4;
  g_ctx = ctx;
  return 0;
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
  const char *u = g_url;

  if (strncmp(u, "http://", 7) == 0)
    u += 7;
  else if (strncmp(u, "https://", 8) == 0)
    u += 8;
  else {
    fail("link must start with http:// or https://");
    return NULL;
  }

  char fname[256];
  {
    char tmp[2048];
    const char *slash = strchr(u, '/');
    snprintf(tmp, sizeof tmp, "%s", slash ? slash : "/");
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

  int rc = net_init();
  if (rc < 0) {
    failc("network init", rc);
    return NULL;
  }

  int tmpl = sceHttp2CreateTemplate(g_ctx, "Ps5Downloadly/1.0", 3, 1);
  if (tmpl < 0) {
    failc("create template", tmpl);
    return NULL;
  }
  sceHttp2SetAutoRedirect(tmpl, 1);
  sceHttp2SetConnectTimeOut(tmpl, 30 * 1000 * 1000);
  g_ssl_err = 0xffffffffu;
  sceHttp2SetSslCallback(tmpl, ssl_cb, NULL);

  int req = sceHttp2CreateRequestWithURL(tmpl, "GET", g_url, 0);
  if (req < 0) {
    sceHttp2DeleteTemplate(tmpl);
    failc("create request", req);
    return NULL;
  }

  rc = sceHttp2SendRequest(req, NULL, 0);
  if (rc < 0) {
    sceHttp2DeleteRequest(req);
    sceHttp2DeleteTemplate(tmpl);
    {
      char m[200];
      snprintf(m, sizeof m, "send request failed (0x%x) ssl verify: 0x%x",
               (unsigned)rc, g_ssl_err);
      fail(m);
    }
    return NULL;
  }

  int status = 0;
  rc = sceHttp2GetStatusCode(req, &status);
  if (rc < 0) {
    sceHttp2DeleteRequest(req);
    sceHttp2DeleteTemplate(tmpl);
    failc("get status", rc);
    return NULL;
  }
  if (status != 200) {
    char m[64];
    snprintf(m, sizeof m, "server returned HTTP %d", status);
    sceHttp2DeleteRequest(req);
    sceHttp2DeleteTemplate(tmpl);
    fail(m);
    return NULL;
  }

  int lres = -1;
  uint64_t len = 0;
  g_total = 0;
  if (sceHttp2GetResponseContentLength(req, &lres, &len) >= 0 && lres == 0)
    g_total = (long long)len;

  char full[512];
  snprintf(full, sizeof full, DEST "%s", fname);
  int fd = open(full, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) {
    sceHttp2DeleteRequest(req);
    sceHttp2DeleteTemplate(tmpl);
    fail("cannot create file in /data/homebrew");
    return NULL;
  }

  size_t bufsz = 256 * 1024;
  char *buf = malloc(bufsz);
  if (!buf) {
    close(fd);
    sceHttp2DeleteRequest(req);
    sceHttp2DeleteTemplate(tmpl);
    fail("out of memory");
    return NULL;
  }

  int err = 0;
  for (;;) {
    int r = sceHttp2ReadData(req, buf, bufsz);
    if (r == 0) break;
    if (r < 0) {
      err = r;
      break;
    }
    if (write(fd, buf, (size_t)r) != r) {
      err = 1;
      break;
    }
    g_done += r;
  }
  free(buf);
  close(fd);
  sceHttp2DeleteRequest(req);
  sceHttp2DeleteTemplate(tmpl);

  if (err == 1) {
    fail("write error (disk full?)");
    return NULL;
  }
  if (err < 0) {
    failc("read", err);
    return NULL;
  }
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
    "<input id=u placeholder='paste link' style='width:100%;font-size:16px;padding:8px'>"
    "<button onclick='go()' style='font-size:18px;padding:10px;margin-top:8px'>Start download</button>"
    "<pre id=s></pre>"
    "<script>"
    "function tick(){fetch('/status').then(function(r){return r.text()}).then(function(t){document.getElementById('s').textContent=t})}"
    "function go(){fetch('/start?u='+encodeURIComponent(document.getElementById('u').value)).then(tick)}"
    "setInterval(tick,1000);tick();"
    "</script>";

static int open_server(void) {
  int srv = socket(AF_INET, SOCK_STREAM, 0);
  if (srv < 0) return -1;
  int one = 1;
  setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
  struct sockaddr_in a;
  memset(&a, 0, sizeof(a));
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = htonl(INADDR_ANY);
  a.sin_port = htons(PORT);
  if (bind(srv, (struct sockaddr *)&a, sizeof(a)) < 0 ||
      listen(srv, 8) < 0) {
    close(srv);
    return -1;
  }
  return srv;
}

static void kill_old(void) {
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) return;
  struct sockaddr_in a;
  memset(&a, 0, sizeof(a));
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  a.sin_port = htons(PORT);
  if (connect(s, (struct sockaddr *)&a, sizeof(a)) == 0) {
    const char *q = "GET /quit HTTP/1.0\r\n\r\n";
    write(s, q, strlen(q));
    char b[64];
    read(s, b, sizeof b);
  }
  close(s);
}

int main(void) {
  int srv = open_server();
  if (srv < 0) {
    kill_old();
    sleep(2);
    srv = open_server();
  }
  if (srv < 0) return 1;

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
    } else if (strncmp(buf, "GET /quit", 9) == 0) {
      reply(c, "text/plain", "bye");
      close(c);
      close(srv);
      _exit(0);
    } else if (strncmp(buf, "GET /status", 11) == 0) {
      static const char *names[] = {"idle", "downloading", "done", "error"};
      char out[768];
      double d = (double)g_done / 1048576.0;
      double t = (double)g_total / 1048576.0;
      int pct = g_total ? (int)(g_done * 100 / g_total) : 0;
      snprintf(out, sizeof out,
               "build: " BUILD "\nstate: %s\nfile: %s\n%.1f / %.1f MB (%d%%)\n%s\n",
               names[g_state], g_name, d, t, pct, g_msg);
      reply(c, "text/plain", out);
    } else {
      reply(c, "text/html", PAGE);
    }
    close(c);
  }
}
