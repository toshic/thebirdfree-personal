
typedef int (*http_parse_cb)(unsigned long size, char *response, void *pv);

int http_req(char *url,http_parse_cb callback, void *pv);
int http_get(char *hostname, unsigned short port, char *location, http_parse_cb callback, void *pv);

