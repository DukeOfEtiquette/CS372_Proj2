#include <iostream>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

int main(int argc, char **argv)
{
  struct sockaddr_in servAddr;
  struct hostent *server;
  
  //Create the client socket
  int client = socket(AF_INET, SOCK_STREAM, 0);

  //Verify socket was created, print message to std with result
  if(client < 0)
  {
      std::cout << "ERROR: Unable to create client socket.\n";
      return 1;
  }else
  {
      std::cout << "Client socket created successfully.\n";
  }

  //Define server info that we are connecting to
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(atoi(argv[2]));

  server = gethostbyname(argv[1]);
  bcopy((char *)server->h_addr, (char *)&servAddr.sin_addr.s_addr, server->h_length);


  //Attempt to make a connection
  int res = connect(client, (struct sockaddr *)&servAddr, sizeof(servAddr));

  //Make sure it connected, otherwise exit
  if(res == 0)
      std::cout << "Connection established on port number: " << argv[2] << std::endl;
  else{
      std::cout << "ERROR: Connection could not be established on port number: " << argv[2] << std::endl;
      close(client);
      return 1;
  }

  close(client);

	return 0;
}
