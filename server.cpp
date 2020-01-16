#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <algorithm>

#define PORT 3000
using namespace std;

extern int errno;

char board[6][7];
char message[250];

void Init_Board(char board[6][7]){
	int i, j;

	for(i = 0; i < 6; i++)
		for(j = 0; j < 7; j++)
			board[i][j] = ' '; //empty board
}

void Update_Board(char color, int column){
	int i, j;

	for(i = 5; i >= 0; i--)
		if(board[i][column] == ' ')
			board[i][column] = color; //put player piece on board
}

int isValid(int column){
	if(column < 1 || column > 7) //column can be just in [1, 7]
		return 3;
	if(board[0][column] != ' ') //full column
		return 2;
	return 1;
}

int isConnect(){
	int i, j;
	for(i = 0; i < 6; i++)
		for(j = 0; j < 7; j++){
			//verify horizontal right
			int nr_pieces = 0;
			if(board[i][j - 1] != board[i][j])
				for(int k = 1; k < 4; k++)
					if((j + k) < 7 && board[i][j] == board[i][j + k])
						nr_pieces++;
			if(nr_pieces == 4)
				return 1;
			//verify horizontal left
			if(board[i][j + 1] != board[i][j])
				for(int k = 1; k < 4; k++)
					if((j - k) >= 0 && board[i][j] == board[i][j - k])
						nr_pieces++;
			if(nr_pieces == 4)
				return 1;
			//verify vertical up
			if(board[i + 1][j] != board[i][j])
				for(int k = 1; k < 4; k++)
					if((i - k) >= 0 && board[i - k][j] == board[i][j])
						nr_pieces++;
			if(nr_pieces == 4)
				return 1;
			//verify vertical down
			if(board[i - 1][j] != board[i][j])
				for(int k = 1; k < 4; k++)
					if((i + k) < 6 && board[i + k][j] == board[i][j])
						nr_pieces++;
			if(nr_pieces == 4)
				return 1;
			//verify diagonal up left
			if(board[i + 1][j + 1] != board[i][j])
				for(int k = 1; k < 4; k++)
					if((i - k) >= 0 && (j - k) >= 0 && board[i - k][j - k] == board[i][j])
						nr_pieces++;
			if(nr_pieces == 4)
				return 1;
			//verify diagonal up right
			if(board[i + 1][j - 1] != board[i][j])
				for(int k = 1; k < 4; k++)
					if((i - k) >= 0 && (j + k) < 7 && board[i - k][j + k] == board[i][j])
						nr_pieces++;
			if(nr_pieces == 4)
				return 1;
			//verify diagonal down left
			if(board[i - 1][j + 1] != board[i][j])
				for(int k = 1; k < 4; k++)
					if((i + k) < 6 && (j - k) >= 0 && board[i + k][j - k] == board[i][j])
						nr_pieces++;
			if(nr_pieces == 4)
				return 1;
			//verify diagonal down right
			if(board[i - 1][j - 1] != board[i][j])
				for(int k = 1; k < 4; k++)
					if((i + k) < 6 && (j + k) < 7 && board[i + k][j + k] == board[i][j])
						nr_pieces++;
			if(nr_pieces == 4)
				return 1;
		}
	return 0;
}

string Convert_Board(char board[6][7]){
	int i, j;
	string message = "";
	for(i = 0; i < 6; i++)
		for(j = 0; j < 7; j++)
			message += board[i][j];
	return message;
}

char * conv_addr(struct sockaddr_in address)
{
    static char str[25];
    char port[7];

    strcpy (str, inet_ntoa (address.sin_addr));
    bzero (port, 7);
    sprintf (port, ":%d", ntohs (address.sin_port));
    strcat (str, port);
    return (str);
}

int Client_Communication(int fd, int verify, char player){
	char buffer[1000];
	int bytes;
	char from_client[100]; 
	char to_client[100]=" ";

	bytes = read (fd, from_client, sizeof(buffer));
	if (bytes < 0)
	{
	  perror ("Error to read() from client.\n");
	  return 0;
	}
	printf ("[server]Message received...%s\n", from_client);
	  
	if(strcmp(from_client, "exit game\n") == 0)
		return -1;

	int column = stoi(from_client);
	printf("%d\n", column);
	if(isValid(column) == 3){
		strcpy(to_client, "Column out of range. Try another one!");
        write(fd, to_client, strlen(to_client));
        return -2;
	}
	if(isValid(column) == 2){
		strcpy(to_client, "Full column. Try another one!");
        write(fd, to_client, strlen(to_client));
        return -2;
	}
	if(isValid(column)){
		Update_Board(player, column);
		if(isConnect()){
			strcpy(to_client, "Congrats! You win the game!");
	        write(fd, to_client, strlen(to_client));
	        strcpy(to_client, Convert_Board(board).c_str());
			write(fd, to_client, strlen(to_client));
	        return -1;
		}
		strcpy(to_client, "Move done. Wait for the other player to make a move!");
		bytes = Convert_Board(board).size();
	}
	
	printf("The client whit this descriptor %d made this move: %d\n", fd, column);
	if(fd % 2 == 0){
		if (bytes && write (fd + 1, Convert_Board(board).c_str(), bytes) < 0)
	    {
	        perror ("[server] Eroare la write() catre client.\n");
	        return 0;
	    }
	    
	    if (strlen(to_client) && write(fd, to_client, strlen(to_client)) < 0){
	    	perror ("[server] Eroare la write() catre client.\n");
	        return 0;
	    }
    }
    else if(fd % 2 == 1){
    	if (bytes && write (fd - 1, Convert_Board(board).c_str(), bytes) < 0)
	    {
	        perror ("[server] Eroare la write() catre client.\n");
	        return 0;
	    }
	    
	    if (strlen(to_client) && write(fd, to_client, strlen(to_client)) < 0){
	    	perror ("[server] Eroare la write() catre client.\n");
	        return 0;
	    }
    }
    verify = 1;
    return bytes;
}

int main(){
	struct sockaddr_in server;
    struct sockaddr_in from;
    fd_set readfds;
    fd_set actfds;
    struct timeval tv;
    int sd, client;
    int optval=1;
    int fd;
    int nfds;
    unsigned int len;

    pid_t childpid;

    int player1_turn;
    int player2_turn;
    int player1_fd;
    int player2_fd;
    char player1_color;
    char player2_color;

    if((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[server] Eroare la socket().\n");
        return errno;
    }

    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));

    bzero (&server, sizeof (server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);

    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        perror ("[server] Eroare la bind().\n");
        return errno;
    }

    if (listen (sd, 10) == -1)
    {
        perror ("[server] Eroare la listen().\n");
        return errno;
    }

    FD_ZERO (&actfds);
    FD_SET (sd, &actfds);

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    nfds = sd;

    printf ("[server] Asteptam la portul %d...\n", PORT);
    fflush (stdout);
    while (1)
    {
        bcopy ((char *) &actfds, (char *) &readfds, sizeof (readfds));

        if (select (nfds+1, &readfds, NULL, NULL, &tv) < 0)
        {
            perror ("[server] Eroare la select().\n");
            return errno;
        }

        if (FD_ISSET (sd, &readfds))
        {
            len = sizeof (from);
            bzero (&from, sizeof (from));

            client = accept (sd, (struct sockaddr *) &from, &len);

            if (client < 0)
            {
                perror ("[server] Eroare la accept().\n");
                continue;
            }

            if (nfds < client)  nfds = client;

            FD_SET (client, &actfds);
            printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n",client, conv_addr (from));
            fflush (stdout);
        }

        if(nfds > 3 and nfds % 2 == 1){
        	if((childpid == fork()) == 0){
        		player1_fd = nfds - 1;
        		player1_turn = 1;
        		player1_color = 'r';
        		player2_fd = nfds;
        		player2_turn = 0;
        		player2_color = 'y';

        		Init_Board(board);
        		printf("Game started between clients with descriptors %d and %d\n", player1_fd, player2_fd);
        		int start_game = 1;
        		while(1){
        			if(player1_turn == 1){
        				int verify = 0;
        				if(player1_fd != sd){
        					if(start_game == 1){
        						int bytes = Convert_Board(board).size();
        						if(bytes && write(player1_fd, Convert_Board(board).c_str(), bytes) < 0){
        							perror ("[server] Error write() to client.\n");
									return 0;
        						}
        						start_game = 0;
        					}
        					if(Client_Communication(player1_fd, verify, player1_color) == -1){
        						close(player1_fd);
                                FD_CLR(player1_fd, &actfds);
                                close(player1_fd + 1);
                                FD_CLR(player1_fd + 1, &actfds);
								exit(0);
        					}
        					else if(verify == 1){
        						player1_turn = 0;
        						player2_turn = 1;
        						verify = 0;
        					}
        				}
        			}
        			else {
        				int verify = 0;
        				if(player1_fd != sd){
        					if(start_game == 1){
        						int bytes = Convert_Board(board).size();
        						if(bytes && write(player1_fd, Convert_Board(board).c_str(), bytes) < 0){
        							perror ("[server] Error write() to client.\n");
									return 0;
        						}
        						start_game = 0;
        					}
        					if(Client_Communication(player1_fd, verify, player1_color)){
        						close(player2_fd);
                                FD_CLR(player2_fd, &actfds);
                                close(player2_fd - 1);
                                FD_CLR(player2_fd - 1, &actfds);
								exit(0);
        					}
        					else if(verify == 1){
        						player1_turn = 1;
        						player2_turn = 0;
        						verify = 0;
        					}
        				}
        			}
        		}
        	}
        }
    }
}