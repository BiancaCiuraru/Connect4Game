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

void Convert_Board(char a[6][7], string s){
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
    char from_server[100];
    string board_message;

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

    if(connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
      perror("[client]Eroare la connect().\n");
      return errno;
    }

    char board[6][7];
    bzero(to_server, 100);
    bzero(from_server, 100);
    if(read(sd, from_server, 100) < 0)
    {
        perror("[client]Eroare la read() de la server.\n");
        return errno;
    }
    printf("%s\n", from_server);

    while(1){
        bzero(to_server, 100);
        bzero(from_server, 100);

        if(read(sd, from_server, 100) < 0)
        {
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }

        if(strcmp(from_server, "It's your turn!") == 0){
            board_message.resize(100);
            if (read (sd, &board_message[0], board_message.size()) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            Convert_Board(board, board_message);
            Print_Board(board);
            cout << endl;
            printf("%s\n", from_server);
            printf("Enter your move: ");
            fflush(stdout);
            read(0, to_server, 100);
            if(write(sd, to_server, strlen(to_server)) <= 0)
            {
              perror ("[client]Eroare la write() spre server.\n");
              return errno;
            }
        }

        if(strcmp(from_server, "Column out of range. Try another one!") == 0 || strcmp(from_server, "Full column. Try another one!") == 0){
            printf("%s\n", from_server);
            printf("Enter your move: ");
            fflush(stdout);
            read(0, to_server, 100);
            if(write(sd, to_server, strlen(to_server)) <= 0)
            {
              perror ("[client]Eroare la write() spre server.\n");
              return errno;
            }
        }
        if(strcmp(from_server, "Move done. Wait for your oponent to make a move!") == 0){
            printf("%s\n", from_server);
            board_message.resize(100);
            if (read (sd, &board_message[0], board_message.size()) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            Convert_Board(board, board_message);
            Print_Board(board);
            cout << endl;
        }
        if(strcmp(from_server, "You win the game") == 0 || strcmp(from_server, "Game over. Player1 win the game") == 0 || strcmp(from_server, "Game over. Player2 win the game") == 0 || strcmp(from_server, "Game over. Player1 left the game") == 0 || strcmp(from_server, "Game over. Player2 left the game") == 0){
            printf("%s\n", from_server);
            break;
        }
    }
    close (sd);
}