#include "utils.h"
void invalid_format() {
  fprintf(stderr, "invalid or missing options\nusage: snc [­l] [­u] [hostname] port\n");
}

int is_numeric(char* input) {
  int len = strlen (input);
  for (int i = 0; i < len; i ++) {
    if (!isdigit(input[i])) {
      return 0;
    }
  }
  return 1;
}

int read_port(int argc, char* argv[], int* port) {
  char* last_entry = argv[argc-1];
  if (is_numeric(last_entry)) {
    int p = atoi(last_entry);
    if (p <= 1024 || p > 65535) {
      fprintf(stderr, "internal error\n");
      return -1;
    } else {
      *port = p;
      return 0;
    }
  } else {
    invalid_format();
    return -1;
  }
}

void error(char* msg) {
  perror(msg);
  exit(0);
}