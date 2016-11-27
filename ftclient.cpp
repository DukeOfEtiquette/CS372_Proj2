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

#define MSG_LEN 512

int main(int argc, char **argv)
{
  //Make sure either 5 or 6 args passed at command line
  if(argc != 5 && argc != 6)
  {
    std::cout << "usage: ./a.out SERVER_HOST SERVER_PORT -l DATA_PORT\nor...\n";
    std::cout << "usage: ./a.out SERVER_HOST SERVER_PORT -g FILE_NAME DATA_PORT\n";
    return 1;
  }

  //Gate serverHost, serverPort and arg since those are the same no matter which arg
  std::string serverHost = argv[1];
  int serverPort = atoi(argv[2]);
  std::string arg = argv[3];
  std::string fileName = "";
  std::string dataPort = "";
  bool bIsGet = false;

  //If listing command then just grab dataPort
  //If get command then grab fileName as well as dataPort, also turn on flag
  //to indicate it is a get request
  if(arg == "-l")
  {
    dataPort =argv[4];
  }else if(arg == "-g"){
    fileName = argv[4];
    dataPort = argv[5];
    bIsGet = true;
  }else{

    std::cout << "usage: Invalid argument used. Use '-l' to get a directory listing\n";
    std::cout << "or us '-g' followed by a file name to transfer a copy of that file locally.\n";
  }

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
  servAddr.sin_port = htons(serverPort);

  server = gethostbyname(serverHost.c_str());
  bcopy((char *)server->h_addr, (char *)&servAddr.sin_addr.s_addr, server->h_length);

  //Attempt to make a connection
  int res = connect(client, (struct sockaddr *)&servAddr, sizeof(servAddr));

  //Make sure it connected, otherwise exit
  if(res == 0)
      std::cout << "Connection established on port number: " << serverPort << std::endl;
  else{
      std::cout << "ERROR: Connection could not be established on port number: " << serverPort << std::endl;
      close(client);
      return 1;
  }

  //Send arg, fileName, and port num to server

  write(client, arg.c_str(), strlen(arg.c_str()));
  if(bIsGet)
    write(client, fileName.c_str(), strlen(fileName.c_str()));
  write(client, dataPort.c_str(), strlen(dataPort.c_str()));

  int msgLen = 0;
  char rcvMsg[MSG_LEN];

  while( (msgLen = recv(client, rcvMsg, MSG_LEN, 0)) > 0 )
  {
    std::cout << rcvMsg << std::endl;
  } 


  //Wait for information to come back and display as we go..

  //Close the client port
  close(client);

	return 0;
}
