#include <iostream>
#include <errno.h>
#include <iterator>
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sstream>

#define MSG_LEN 512

void printFileList(std::string);
void ReceiveListRequest(int);
void ReceiveFileRequest(int);

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
    dataPort = argv[4];
  }else if(arg == "-g"){
    fileName = argv[4];
    dataPort = argv[5];
    bIsGet = true;
  }else{

    std::cout << "usage: Invalid argument used. Use '-l' to get a directory listing\n";
    std::cout << "or us '-g' followed by a file name to transfer a copy of that file locally.\n";

    return 1;
  }

  struct sockaddr_in servAddr;
  struct sockaddr_in cliAddr;
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

  //Begin setting up data socket to receive request on
  //I established the port before sending the message so that
  //once the message is fully sent I can immediately start listening on
  //the data port to make sure the server doesn't somehow make a 
  //request before data socket is waiting
  int dataSock = socket(AF_INET, SOCK_STREAM, 0);

  //Verify socket was created, print message to std with result
  if(dataSock < 0)
  {
      std::cout << "ERROR: Unable to create data socket.\n";
      return 1;
  }else
  {
      std::cout << "Data socket created successfully.\n";
  }

  //Define data socket info that we are listening on
  cliAddr.sin_family = AF_INET;

  // automatically be filled with current host's IP address
  cliAddr.sin_addr.s_addr = INADDR_ANY; 
  cliAddr.sin_port = htons(atoi(dataPort.c_str()));

  res = bind(dataSock, (struct sockaddr*) &cliAddr, sizeof(cliAddr));

  //Make sure it connected, otherwise exit
  if(res == 0)
      std::cout << "Connection established on port number: " << dataPort << std::endl;
  else{
      std::cout << "ERROR: Connection could not be established on port number: " << dataPort << std::endl;
      close(dataSock);
      return 1;
  }

  //Begin process of sending request message to server
  std::string sndMsg = "";

  //Format message with '%' at the end of each element so server can split msg easily
  sndMsg = arg + "%";

  //If this is a get request, send file name
  if(bIsGet)
  {
	  sndMsg += fileName + "%";
  }

  sndMsg += dataPort;

  //Send message
  write(client, sndMsg.c_str(), strlen(sndMsg.c_str()));

  //Wait for a connection onto the data socket now that message has been sent
  listen(dataSock, 1);

  struct sockaddr_in cli_addr;
  socklen_t cliLen = sizeof(cli_addr);

  //Accept the connection and record requesters info
  int newDataSock = accept(dataSock, (struct sockaddr*) &cli_addr, &cliLen);

  if(newDataSock < 0)
  {
    std::cout << "Error on accepting data connection request.\n";
    close(newDataSock);
    return 1;
  }
  else{
    std::cout << "Data connection established with server.\n";
  }

  if(bIsGet)
  {
    ReceiveFileRequest(newDataSock);

  }else if(!bIsGet){
    ReceiveListRequest(newDataSock);

  }

  //Close all sockets
  close(client);
  close(dataSock);
  close(newDataSock);

	return 0;
}

void printFileList(std::string files)
{
  std::istringstream iss(files);

  std::cout << "\nDirectory List: \n";

  //Stream one item at a time, packet elements delim on space
  do{
    std::string file;
    iss >> file;
    std::cout << file;

	//New line for new file
	std::cout << std::endl;
  }while(iss);
}

void ReceiveListRequest(int dataSock)
{
  int msgLen = 0;
  char rcvMsg[MSG_LEN];
  std::string files = "";

  bzero(rcvMsg, MSG_LEN);

  //Keep reading all data
  while( (msgLen = read(dataSock, rcvMsg, sizeof(rcvMsg))) > 0)
  {
	  files += rcvMsg;
  }

  //Print the directory list
  printFileList(files);

}

void ReceiveFileRequest(int dataSock)
{
  int msgLen = 0;
  char rcvMsg[MSG_LEN];

  bzero(rcvMsg, MSG_LEN);

  while( (msgLen = read(dataSock, rcvMsg, sizeof(rcvMsg))) > 0)
  {

  }

}
