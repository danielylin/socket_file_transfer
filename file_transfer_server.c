#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {

    setbuf(stdout, NULL);
 
    printf("Enter a port number: ");
    int check_portno = scanf("%d", &portno);

    if(check_portno != 1){
      printf("\n Port number error. \n")
    }

    printf("\n Enter an output file: ");
    int check_file = scanf("%s", filename);

    if(check_file != 1){
      printf("\n Filename error. \n")
    }

  struct addrinfo hints, *results, *record; // Socket address structure for server.

  memset(&hints, 0, sizeof(hints)); // Initialize the memory block with 0's.
  hints.ai_family = AF_UNSPEC; // To be IP-agnostic.
  hints.ai_socktype = SOCK_STREAM; // TCP stream.
  hints.ai_flags = AI_PASSIVE;  //assign the address of my local host to the socket.

  // Alter portno to string format to pass into getaddrinfo.
  int port_size = snprintf(NULL, 0, "%d", portno) + 1; // Size of the port number and null-terminator.
  char *portno_str = malloc(port_size); // Allocating memory to port number string.
  snprintf(portno_str, port_size, "%d", portno); // Casts portno to a string.

  // Call getaddrinfo to translate the address to be callable by socket().
  int status;
  if((status = getaddrinfo(NULL, portno_str, &hints, &results)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }
  fprintf(stdout, "Server socket translated. \n"); // Results now points to a linked list of addrinfos.

  free(portno_str); // free memory

  int server_socket;

  // Loop through the linked list of records.
  for(record = results; record != NULL; record = record->ai_next) {
    server_socket = socket(record->ai_family, record->ai_socktype, record->ai_protocol);
    if (server_socket == -1) {
      fprintf(stderr,"Error with opening socket.");
      continue;
    }  

    int enable = 1;

    // To prevent "address in use" error message.
    if((setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) == -1) {
      fprintf(stderr,"Error with setsockopt.");
      exit(1);
    }

    if(bind(server_socket, record->ai_addr, record->ai_addrlen) == -1) {
      close(server_socket);
      fprintf(stderr,"Binding failed.");
      exit(1);
      continue; // If there is a binding error, try loop again.
    }

    break; // If we successfully connect then end the loop.
  }

  freeaddrinfo(results);

  if(record == NULL) {
    fprintf(stderr, "Server failed to bind.\n");
    exit(1);
  }

  fprintf(stdout, "Server socket created and bound.\n");

  if(listen(server_socket, 5) == -1) {                                                                              // Start server socket listen
        fprintf(stderr, "Listening failed.");
        exit(1);
  }

  fprintf(stdout, "Server waiting for connections...\n");

  int client_socket; 
  struct sockaddr client_address;
  socklen_t client_address_size;

  while(1) {
    client_address_size = sizeof(client_address);
    client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_size);

    if(client_socket == -1) {
      continue; 
    }
    
    if(!fork()) {
      close(server_socket); // child process doesn't need the listener
      FILE *out_file = fopen(filename, "a+"); // Open the file. 
      if(out_file == NULL) {
        fprintf(stderr, "Failed to open file.");
        exit(1);
      }

      fseek(out_file, 0, SEEK_END); // seek to end of file
      int file_size = ftell(out_file); // get current file pointer
      fseek(out_file, 0, SEEK_SET); // seek back to beginning of file

      ssize_t total_transferred_bytes, transferred_bytes;

      char buffer[file_size];
      memset(buffer, 0, sizeof(buffer));

      fread(buffer, sizeof(buffer), 1, out_file); 

      if(sizeof(buffer) < file_size){
        fprintf(stderr, "Error reading bytes from file.");
        exit(1);
      }

      total_transferred_bytes = 0;

      while(total_transferred_bytes < sizeof(buffer)) {
        if((transferred_bytes = send(client_socket, buffer, sizeof(buffer), 0)) == -1) {
          fprintf(stderr, "Failed to send file.");
          exit(1);
        } 
        total_transferred_bytes += transferred_bytes;
      }
      close(client_socket);
    }
    close(client_socket);
  }
}
