#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <fstream>

extern int errno;

using namespace std;
int port; 

void Server_Communication(char a[6][7], string s) {
    int i, j, k = 0;
    for(i = 0; i < 6; i++) {
        for(j = 0; j < 7; j++)
            a[i][j] = s[k++];
    }
}


void Print_Board(char A[7][7]){
    int i, j;

    for(i = 0; i < 6; i++) {
        for(j = 0; j < 7; j++)
            cout << "|" << A[i][j];
        cout << "|" << endl;
    }
    cout << "================" << endl;

    for(j = 1; j <= 7; j++) 
    	cout << "|" << j;
    cout << "|"<<endl;
}

int main(int argc, char *argv[]){
	int sd;
	struct sockaddr_in server;
	char to_server[100];

	if(argc != 3)
	{
	  printf ("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
	  return -1;
	}

	port = atoi (argv[2]);

	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
	{
	  perror ("[client] Eroare la socket().\n");
	  return errno;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_port = htons (port);

	if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
	{
	  perror ("[client]Eroare la connect().\n");
	  return errno;
	}

	char board[6][7];
	string s;
	s.resize(100);
	if(read (sd, &s[0], s.size()) < 0)
    {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
    }
    Server_Communication(board, s);
    Print_Board(board);
    cout << endl;
    while(1) {

        bzero (to_server, 100);
        printf ("[client]Introduceti mutarea dorita: ");
        fflush (stdout);
        read(0, to_server, 100);
        if (write(sd, to_server, 100) <= 0)
        {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
        }

        if(strcmp(to_server, "exit game\n") == 0)
        	break;

        if(read(sd, to_server, 100) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }
        cout << to_server << endl;
        if(strcmp(to_server, "Column out of range. Try another one!") == 0 || strcmp(to_server, "Full column. Try another one!") == 0){
        	printf("%s\n", to_server);
        	s.resize(100);
		    if (read (sd, &s[0], s.size()) < 0)
		    {
		        perror ("[client]Eroare la read() de la server.\n");
		        return errno;
		    }
		    Server_Communication(board, s);
		    Print_Board(board);
		    cout << endl;
        }
    }
    close(sd);
}