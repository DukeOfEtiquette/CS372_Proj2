/* Author: Adam DuQuette
 * 
 * Beej's guide was used a lot as reference for this project.
 * I also used the following guide for the C++ bits:
 * http://www.bogotobogo.com/cplusplus/sockets_server_client.php
 *
 * I also used Python and C++ standard documention
 *
 * In places I found code ideas from StackOverflow or other places
 * I have made a note of it.
 *
 * */


#include <iostream>
#include <fstream>
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
void ReceiveFileRequest(int, std::string);
bool fileExist(const char *);

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

  //Make appropriate function call based on user arg input
  if(bIsGet)
  {
    ReceiveFileRequest(newDataSock, fileName);
  }else if(!bIsGet){
    ReceiveListRequest(newDataSock);
  }

  //Close all sockets
  close(client);
  close(dataSock);
  close(newDataSock);

	return 0;
}

//Simple helper function to print the list of files to stdout
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

//Function to handle all file listing requests
void ReceiveFileRequest(int dataSock, std::string fileName)
{
  int msgLen = 0;
  char rcvMsg[MSG_LEN];

  int nCopy = 1;

  //Check if the file already exists locally
  //If it does then add a unique number to the end of the file
  //to show it is a copy
  while(fileExist(fileName.c_str()))
  {
    std::ostringstream st;
    st << nCopy;

    //If this is the first iteration then insert a 1 at the end of the file
    //name but before the file extension
    //Else replae whatever number is there with whatever iteration it is on
    if(nCopy == 1)
      fileName.insert(fileName.length() - 4, st.str());
    else
      fileName.replace(fileName.length() - 5, 1, st.str());

    //Inc counter
    nCopy++;
  }

  //Open file we are going to write to
  std::ofstream newFile;
  newFile.open(fileName.c_str());

  bzero(rcvMsg, MSG_LEN);

  //Receive messages and write them to the file
  while( (msgLen = read(dataSock, rcvMsg, sizeof(rcvMsg))) > 0)
    newFile << rcvMsg;

}

//Simple helper function to determine if a file exists at the given path
//Found solution from here:
//http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
bool fileExist(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}
