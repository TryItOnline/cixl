#include <arpa/inet.h>
#include "cixl/net.h"

uint32_t cx_host32(uint32_t v) { return ntohl(v); }

uint32_t cx_net32(uint32_t v) { return htonl(v); }
