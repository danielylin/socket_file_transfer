#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFSIZE 1024

/* Main ========================================================= */
int main(int argc, char **argv) {

    setbuf(stdout, NULL);
 
    printf("Enter a port number: ");
    int check_portno = scanf("%d", &portno);

    if(check_portno != 1){
      printf("\n Port number error. \n")
    }

    printf("\n Enter server name: ");
    int check_servername = scanf("%s", hostname);

    if(check_servername != 1){
      printf("\n Server name error. \n")
    }

    printf("\n Enter an output file (Default is hello.txt): ");
    int check_file = scanf("%s", filename);

    if(check_file != 1){
      printf("\n File name error. \n")
    }

    struct addrinfo hints, *results, *record; // Socket address structure for the server.

    hints.ai_family = AF_UNSPEC; // To be IP-agnostic.
    hints.ai_socktype = SOCK_STREAM; // TCP server.
    //hints.ai_flags = AI_PASSIVE; // Assign the address of my local host to the socket.

    memset(&hints, 0, sizeof(struct addrinfo)); // Fill address of hints with 0.

    // Alter portno to string format to pass into getaddrinfo.
    int port_length = snprintf(NULL, 0, "%d", portno) + 1;
    char *portno_str = malloc(port_length); // Allocating memory to a pointer to portno_str. 
    sprintf(portno_str, "%d", portno); // Cast portno to string.

    // Call getaddrinfo to translate the address to be callable by socket().
    int status;
    if((status = getaddrinfo(hostname, portno_str, &hints, &results)) != 0) {
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      exit(1);
    }

    free(portno_str); // free memory

    int client_socket;
    
    for(record = results; record != NULL; record = record->ai_next) {
      client_socket = socket(record->ai_family, record->ai_socktype, record->ai_protocol);
      if (client_socket == -1) {
        fprintf(stderr, "Error with opening socket.\n");
        continue;
      }  

      if(connect(client_socket, record->ai_addr, record->ai_addrlen) ==-1) {
        close(client_socket);
        fprintf(stderr, "Error when connecting socket.\n");
      }
      break; // If we successfully connect then end the loop.
    }

    if (record == NULL) {
      fprintf(stderr, "Client failed to connect.\n");
      exit(1);
    }

    freeaddrinfo(results);

    FILE *in_file = fopen(filename, "w");

    if(in_file == NULL) {
      fprintf(stderr, "Failed to open file.");
      exit(1);
    }

    ssize_t read_bytes = 0;
    char buffer[BUFSIZE];
    memset(buffer, 0, sizeof(buffer));

    while((read_bytes = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
      fwrite(buffer, read_bytes, sizeof(char), in_file);
    }

    if(read_bytes == -1) {
      fprintf(stderr, "Error reading file.");
      exit(1);
    }
    fclose(in_file);
  }