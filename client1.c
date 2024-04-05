#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>

#define SERVER_IP "127.0.0.1"
#define PORT1 4001
#define PORT2 4002
#define PORT3 4003
#define DELAY_US 100000


typedef struct {
  float out1;
  float out2;
  float out3;
  long long timestamp;
} server_data_t;

int create_socket(int port) {
  int sockfd;
  struct sockaddr_in server_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket creation failed");
    exit(1);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
  server_addr.sin_port = htons(port);

  if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("connection failed");
    exit(1);
  }

  // Set socket to non-blocking mode
  if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
    perror("fcntl failed");
    exit(1);
  }

  return sockfd;
}

int receive_data(int sockfd, float *value) {
  char buffer[16];
  int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
  if (bytes_received == 0) {
    return 0;  // Connection closed
  } else if (bytes_received < 0) {
    if (errno == EWOULDBLOCK) {
      return -1;  // No data available yet
    }
  }
  buffer[bytes_received] = '\0';
  *value = atof(buffer);
  return 1;
}

long long get_current_micro() {
    struct timespec now;
    timespec_get(&now, TIME_UTC); 
    return now.tv_sec * 1000000LL + now.tv_nsec / 1000LL;
}

void print_json(server_data_t* data) {
  printf("{\"timestamp\": %lld, \"out1\": ", data->timestamp);
  if (data->out1 == 0.0) {
    printf("\"--\"");
  } else {
    printf("\"%.1f\"", data->out1);
  }
  printf(", \"out2\": ");
  if (data->out2 == 0.0) {
    printf("\"--\"");
  } else {
    printf("\"%.1f\"", data->out2);
  }
  printf(", \"out3\": ");
  if (data->out3 == 0.0) {
    printf("\"--\"");
  } else {
    printf("\"%.1f\"", data->out3);
  }
  printf("}\n");
}

int main() {
  int sockfd1, sockfd2, sockfd3;
  server_data_t data;
  float value = 0.0;
  long long  remaining_time;

  sockfd1 = create_socket(PORT1);
  sockfd2 = create_socket(PORT2);
  sockfd3 = create_socket(PORT3);

  while (1) {
    
    data.timestamp =  get_current_micro() / 1000;
    if (receive_data(sockfd1, &value) > 0) {
      data.out1 = value;
      //value = 0.0;
    } else {
      data.out1 = 0.0;
    }
    if (receive_data(sockfd2, &value) > 0) {
      data.out2 = value;
      //value = 0.0;
    }
    else {
      data.out2 = 0.0;
    }
    if (receive_data(sockfd3, &value) > 0) {
      data.out3 = value;
      //value = 0.0;
    } else {
      data.out3 = 0.0;
    }
    print_json(&data);
    
  
    remaining_time = DELAY_US - (get_current_micro() - data.timestamp *1000);
    usleep(remaining_time);
  }
  return 0;
}
