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

#define PORT 3000
using namespace std;

extern int errno;

char board[6][7];
char message[250];
int change_turn = 0;

void Init_Board(char board[6][7]){
	int i, j;

	for(i = 0; i < 6; i++)
		for(j = 0; j < 7; j++)
			board[i][j] = ' '; //empty board
}

void Update_Board(char color, int column){
	int i = 5;

	while(board[i][column - 1] != ' ' and i >= 0)
		i--;
	board[i][column - 1] = color; //put player piece on board
}

int isValid(int column){
	if(column < 1 || column > 7) //column can be just in [1, 7]
		return 3;
	if(board[0][column - 1] != ' ') //full column
		return 2;
	return 1;
}

int isConnect(){
	int i, j;
	for(i = 0; i < 6; i++)
		for(j = 0; j < 7; j++){
			if(board[i][j] != ' '){
				//verify horizontal right
				int nr_pieces = 1;
				if(board[i][j - 1] != board[i][j] || (j - 1) < 0)
					for(int k = 1; k < 4; k++)
						if((j + k) < 7 && board[i][j] == board[i][j + k])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify horizontal left
				nr_pieces = 1;
				if(board[i][j + 1] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((j - k) >= 0 && board[i][j] == board[i][j - k])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify vertical up
				nr_pieces = 1;
				if(board[i + 1][j] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((i - k) >= 0 && board[i - k][j] == board[i][j])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify vertical down
				nr_pieces = 1;
				if(board[i - 1][j] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((i + k) < 6 && board[i + k][j] == board[i][j])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify diagonal up left
				nr_pieces = 1;
				if(board[i + 1][j + 1] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((i - k) >= 0 && (j - k) >= 0 && board[i - k][j - k] == board[i][j])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify diagonal up right
				nr_pieces = 1;
				if(board[i + 1][j - 1] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((i - k) >= 0 && (j + k) < 7 && board[i - k][j + k] == board[i][j])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify diagonal down left
				nr_pieces = 1;
				if(board[i - 1][j + 1] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((i + k) < 6 && (j - k) >= 0 && board[i + k][j - k] == board[i][j])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify diagonal down right
				nr_pieces = 1;
				if(board[i - 1][j - 1] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((i + k) < 6 && (j + k) < 7 && board[i + k][j + k] == board[i][j])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
			}
		}
	return 0;
}

string Convert_Board(char board[6][7]){
	int i, j;
	string message = ""; 
	for(i = 0; i < 6; i++)  //convert board into string for send this to the client
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

int Client_Communication(int fd, char color){
	char buffer[1000];
	int bytes;
	char from_client[100];
	char to_client[200]=" ";

	change_turn = 0;
	bzero(from_client, 200);
	bytes = read(fd, from_client, sizeof (buffer));
	if (bytes < 0)
	{
	  perror ("Eroare la read() de la client.\n");
	  return 0;
	}
	printf ("[server]Mesajul a fost receptionat...%s\n", from_client);
      
    if(strcmp(from_client, "exit game\n") == 0){ //player exit game
    	strcpy(to_client, "You left the game");
    	write(fd, to_client, strlen(to_client));
    	if(fd%2 == 0){
    		strcpy(to_client, "Game over. Player1 left the game"); //send message to oponent
    		write(fd + 1, to_client, strlen(to_client));
    	}
    	else {
    		strcpy(to_client, "Game over. Player2 left the game"); //send message to oponent
    		write(fd - 1, to_client, strlen(to_client));
    	}
    	return -1;
    }

    if(isdigit(from_client[0])){
		int column = atoi(from_client);
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
			Update_Board(color, column);
			if(isConnect()){
				strcpy(to_client, "You win the game. Do you want to play a new reprise? Type [yes/no]");
		   		write(fd, to_client, strlen(to_client));
		   		read(fd, from_client, 100);
		    	if(strcmp(from_client, "yes\n") == 0){ //player win and accept new reprise
		    		if(fd%2 == 0){ //is first player
			    		strcpy(to_client, "Game over. Player1 win the game and he want to play again. Do you want to play a new reprise? Type [yes/no]");
			    		write(fd + 1, to_client, strlen(to_client));
			    		read(fd + 1, from_client, 100);
			    		if(strcmp(from_client, "yes\n") == 0){ //oponent accept new reprise
			    			strcpy(to_client, "Your oponent want a new game. Start Game!"); 
		   					write(fd, to_client, strlen(to_client));
			    			return -3;
			    		}
			    		else{ //oponent refuse new reprize
			    			strcpy(to_client, "Your oponent don't want a new game. Game over!");
		   					write(fd, to_client, strlen(to_client));
		   					strcpy(to_client, "Game Over!");
		   					write(fd + 1, to_client, strlen(to_client));
			    			return -4;
			    		}
			    	}
			    	else { //is second player
			    		strcpy(to_client, "Game over. Player2 win the game and he want to play again. Do you want to play a new reprise? Type [yes/no]");
			    		write(fd - 1, to_client, strlen(to_client));
			    		read(fd - 1, from_client, 100);
			    		if(strcmp(from_client, "yes\n") == 0){
			    			strcpy(to_client, "Your oponent want a new game. Start Game!");
		   					write(fd, to_client, strlen(to_client));
			    			return -3;
			    		}
			    		else{
			    			strcpy(to_client, "Your oponent don't want a new game. Game over!");
		   					write(fd, to_client, strlen(to_client));
		   					strcpy(to_client, "Game Over!");
		   					write(fd - 1, to_client, strlen(to_client));
			    			return -4;
			    		}
			    	}
		    	}
		    	else{ //player win and refuse new reprise
		    		strcpy(to_client, "Game Over!");
		   			write(fd, to_client, strlen(to_client));
		    		if(fd%2 == 0){ //send message to oponent
			    		strcpy(to_client, "Game over. Player1 win the game and he don't want to play again. Game over!");
			    		write(fd + 1, to_client, strlen(to_client));
			    	}
			    	else {
			    		strcpy(to_client, "Game over. Player2 win the game and he don't want to play again. Game over!");
			    		write(fd - 1, to_client, strlen(to_client));
			    	}
			    	return -4;
		    	}
			}
			strcpy(to_client, "Move done. Wait for your oponent to make a move!"); //valid move, send message
			write(fd, to_client, strlen(to_client));

			bytes = Convert_Board(board).size();
			if(bytes && write(fd, Convert_Board(board).c_str(), bytes) < 0){ //send updated board
		        perror ("[server] Eroare la write() catre client.\n");
		        return 0;
		    }
		    change_turn = 1;
		}
	}
	else {
		strcpy(to_client, "Command not found. Try another one!"); //command is not for exit or for move or for new reprise
        write(fd, to_client, strlen(to_client));
        return -2;
	}
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
	pid_t child;
	char message[250];
	int clients[250] = {0};

	int player1_fd;
	int player1_turn;
	int player1_color;
	int player2_fd;
	int player2_turn;
	int player2_color;
	int player1_scor;
	int player2_scor;
	int nr_reprize;

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("[server] Eroare la socket().\n");
		return errno;
	}

	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	bzero(&server, sizeof (server));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl (INADDR_ANY);
	server.sin_port = htons (PORT);

	if(bind(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1)
	{
		perror ("[server] Eroare la bind().\n");
		return errno;
	}

	if(listen(sd, 5) == -1)
	{
		perror ("[server] Eroare la listen().\n");
		return errno;
	}

	FD_ZERO (&actfds);
	FD_SET (sd, &actfds);

	tv.tv_sec = 1;
	tv.tv_usec = 0;

	nfds = sd;

	printf("[server] Asteptam la portul %d...\n", PORT);
	fflush(stdout);
	   
	while(1)
	{
		bcopy((char *) &actfds, (char *) &readfds, sizeof(readfds));

		if(select(nfds+1, &readfds, NULL, NULL, &tv) < 0)
		{
			perror("[server] Eroare la select().\n");
			return errno;
		}

		if(FD_ISSET(sd, &readfds))
		{
			len = sizeof(from);
			bzero(&from, sizeof(from));

			client = accept(sd, (struct sockaddr *) &from, &len);
			if(client < 0)
			{
				perror("[server] Eroare la accept().\n");
				continue;
			}

		  	if (nfds < client)
		    	nfds = client;
		    
			FD_SET(client, &actfds);

			printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n", client, conv_addr(from));
			fflush(stdout);
		}

		if(nfds%2 == 1 && nfds > 3 && clients[nfds] == 0 && clients[nfds - 1] == 0){ //nfds starts from 4
			clients[nfds] = clients[nfds - 1] = 1; //this clients were visited

			if((child = fork() == 0)){ //start process
				player1_fd = nfds - 1; //set players descriptors
        		player2_fd = nfds;
        		player1_scor = 0;
        		player2_scor = 0;
        		nr_reprize = 0;

        		int new_reprise = 1;
        		while(new_reprise){
        			int i = rand()%2; //random turn and color for each player
					player1_turn = i;
					player2_turn = (i + 1)%2;

					i = rand()%2;
					if(i){
						player1_color = 'r';
						player2_color = 'y';
					}else{
						player1_color = 'y';
						player2_color = 'r';
					}

        			Init_Board(board);
	        		strcpy(message, "You are the first player");
					write(player1_fd, message, strlen(message));
	        		strcpy(message, "You are the second player. Wait for your oponent to make move!");
					write(player2_fd, message, strlen(message));
	        		
	        		printf("Game started between clients with descriptors %d and %d\n", player1_fd, player2_fd);
	        		close(sd);
	        		while(1){
	        			if(player1_turn == 1){ //is his turn
	        				if(player1_fd != sd){ //different from socket 
	        					bzero(message, 100);
	        					strcpy(message, "It's your turn!");
	        					write(player1_fd, message, strlen(message)); //send turn message + board
	        					int bytes = Convert_Board(board).size(); 
								if(bytes && write(player1_fd, Convert_Board(board).c_str(), bytes) < 0){
									perror ("[server] Error write() to client.\n");
									return 0;
								}
								int resp = Client_Communication(player1_fd, player1_color);
	        					if(resp == -1){ //exit game
	        						close(player1_fd);
	                                FD_CLR(player1_fd, &actfds);
	                                close(player2_fd);
	                                FD_CLR(player2_fd, &actfds);
	                                clients[player1_fd] = clients[player2_fd] = 0;
									exit(0);
	        					}
	        					else if(resp == -3){ //win with new reprise
	        						player1_scor++;
	        						nr_reprize++;
	        						break;
	        					}
	        					else if(resp == -4){ //win without new reprise
	        						close(player1_fd);
	                                FD_CLR(player1_fd, &actfds);
	                                close(player2_fd);
	                                FD_CLR(player2_fd, &actfds);
	                                clients[player1_fd] = clients[player2_fd] = 0;
									new_reprise = 0;
									exit(0);
	        					}
	        					else if(change_turn){ //pass turn to oponent
	        						player1_turn = 0;
	        						player2_turn = 1;
	        						change_turn = 0;
	        					}
	        				}
	        			}
	        			else {
	        				if(player2_fd != sd){
	        					bzero(message, 100);
	        					strcpy(message, "It's your turn!");
	        					write(player2_fd, message, strlen(message));
	        					int bytes = Convert_Board(board).size();
								if(bytes && write(player2_fd, Convert_Board(board).c_str(), bytes) < 0){
									perror ("[server] Error write() to client.\n");
									return 0;
								}
								int resp = Client_Communication(player2_fd, player2_color);
	        					if(resp == -1){
	        						close(player2_fd);
	                                FD_CLR(player2_fd, &actfds);
	                                close(player1_fd);
	                                FD_CLR(player1_fd, &actfds);
	                                clients[player1_fd] = clients[player2_fd] = 0;
									exit(0);
	        					}
	        					else if(resp == -3){ //new reprise
	        						player2_scor++;
	        						nr_reprize++;
	        						break;
	        					}
	        					else if(resp == -4){
	        						close(player2_fd);
	                                FD_CLR(player2_fd, &actfds);
	                                close(player1_fd);
	                                FD_CLR(player1_fd, &actfds);
	                                clients[player1_fd] = clients[player2_fd] = 0;
									new_reprise = 0;
									exit(0);
	        					}
	        					else if(change_turn){
	        						player2_turn = 0;
	        						player1_turn = 1;
	        						change_turn = 0;
	        					}
	        				}
	        			}
	        		}
        		}
			}
		}
	}			
}