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
				int nr_pieces = 0;
				if(board[i][j - 1] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((j + k) < 7 && board[i][j] == board[i][j + k])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify horizontal left
				nr_pieces = 0;
				if(board[i][j + 1] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((j - k) >= 0 && board[i][j] == board[i][j - k])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify vertical up
				nr_pieces = 0;
				if(board[i + 1][j] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((i - k) >= 0 && board[i - k][j] == board[i][j])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify vertical down
				nr_pieces = 0;
				if(board[i - 1][j] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((i + k) < 6 && board[i + k][j] == board[i][j])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify diagonal up left
				nr_pieces = 0;
				if(board[i + 1][j + 1] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((i - k) >= 0 && (j - k) >= 0 && board[i - k][j - k] == board[i][j])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify diagonal up right
				nr_pieces = 0;
				if(board[i + 1][j - 1] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((i - k) >= 0 && (j + k) < 7 && board[i - k][j + k] == board[i][j])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify diagonal down left
				nr_pieces = 0;
				if(board[i - 1][j + 1] != board[i][j])
					for(int k = 1; k < 4; k++)
						if((i + k) < 6 && (j - k) >= 0 && board[i + k][j - k] == board[i][j])
							nr_pieces++;
				if(nr_pieces == 4)
					return 1;
				//verify diagonal down right
				nr_pieces = 0;
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

int change_turn = 0;

int Client_Communication(int fd, char color){
	char buffer[1000];
	int bytes;
	char from_client[100];
	char to_client[100]=" ";

	change_turn = 0;
	bytes = read(fd, from_client, sizeof (buffer));
	if (bytes < 0)
	{
	  perror ("Eroare la read() de la client.\n");
	  return 0;
	}
	printf ("[server]Mesajul a fost receptionat...%s\n", from_client);
      
    if(strcmp(from_client, "exit game\n") == 0){
    	if(fd%2 == 0){
    		strcpy(to_client, "Game over. Player1 left the game");
    		write(fd + 1, to_client, strlen(to_client));
    	}
    	else {
    		strcpy(to_client, "Game over. Player2 left the game");
    		write(fd - 1, to_client, strlen(to_client));
    	}
    	return -1;
    }

    if(isdigit(from_client[0])){
		int column = atoi(from_client);
		if(isValid(column) == 3){
			strcpy(to_client, "Column out of range. Try another one!");
			printf("%s\n", to_client);
	        write(fd, to_client, strlen(to_client));
	        return -2;
		}
		if(isValid(column) == 2){
			strcpy(to_client, "Full column. Try another one!");
			printf("%s\n", to_client);
	        write(fd, to_client, strlen(to_client));
	        return -2;
		}
		if(isValid(column)){
			Update_Board(color, column);
			if(isConnect()){
				strcpy(to_client, "You win the game");
		   		write(fd, to_client, strlen(to_client));
				if(fd%2 == 0){
		    		strcpy(to_client, "Game over. Player1 win the game");
		    		write(fd + 1, to_client, strlen(to_client));
		    	}
		    	else {
		    		strcpy(to_client, "Game over. Player2 win the game");
		    		write(fd - 1, to_client, strlen(to_client));
		    	}
		    	return -1;
			}
			strcpy(to_client, "Move done. Wait for your oponent to make a move!");
			write(fd, to_client, strlen(to_client));

			bytes = Convert_Board(board).size();
			if(bytes && write(fd, Convert_Board(board).c_str(), bytes) < 0){ //send updated board
		        perror ("[server] Eroare la write() catre client.\n");
		        return 0;
		    }
		    change_turn = 1;
		}
	}
	return bytes;
}

int main(){
	struct sockaddr_in server;	/* structurile pentru server si clienti */
	struct sockaddr_in from;
	fd_set readfds;		/* multimea descriptorilor de citire */
	fd_set actfds;		/* multimea descriptorilor activi */
	struct timeval tv;		/* structura de timp pentru select() */
	int sd, client;		/* descriptori de socket */
	int optval=1; 			/* optiune folosita pentru setsockopt()*/ 
	int fd;			/* descriptor folosit pentru 
				   parcurgerea listelor de descriptori */
	int nfds;			/* numarul maxim de descriptori */
	unsigned int len;			/* lungimea structurii sockaddr_in */
	pid_t child;
	char message[250];
	int clients[250] = {0};

	int player1_fd;
	int player1_turn;
	int player1_color;
	int player2_fd;
	int player2_turn;
	int player2_color;

	/* creare socket */
	if((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
	{
	  perror ("[server] Eroare la socket().\n");
	  return errno;
	}

	/*setam pentru socket optiunea SO_REUSEADDR */ 
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));

	/* pregatim structurile de date */
	bzero (&server, sizeof (server));

	/* umplem structura folosita de server */
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl (INADDR_ANY);
	server.sin_port = htons (PORT);

	/* atasam socketul */
	if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
	{
	  perror ("[server] Eroare la bind().\n");
	  return errno;
	}

	/* punem serverul sa asculte daca vin clienti sa se conecteze */
	if (listen (sd, 5) == -1)
	{
	  perror ("[server] Eroare la listen().\n");
	  return errno;
	}

	/* completam multimea de descriptori de citire */
	FD_ZERO (&actfds);		/* initial, multimea este vida */
	FD_SET (sd, &actfds);		/* includem in multime socketul creat */

	tv.tv_sec = 1;		/* se va astepta un timp de 1 sec. */
	tv.tv_usec = 0;

	/* valoarea maxima a descriptorilor folositi */
	nfds = sd;

	printf ("[server] Asteptam la portul %d...\n", PORT);
	fflush (stdout);
	    
	/* servim in mod concurent clientii... */
	while (1)
	{
	  /* ajustam multimea descriptorilor activi (efectiv utilizati) */
		bcopy ((char *) &actfds, (char *) &readfds, sizeof (readfds));

		/* apelul select() */
		if (select (nfds+1, &readfds, NULL, NULL, &tv) < 0)
		{
		perror ("[server] Eroare la select().\n");
		return errno;
		}
		/* vedem daca e pregatit socketul pentru a-i accepta pe clienti */
		if (FD_ISSET (sd, &readfds))
		{
		/* pregatirea structurii client */
		len = sizeof (from);
		bzero (&from, sizeof (from));

		/* a venit un client, acceptam conexiunea */
		client = accept (sd, (struct sockaddr *) &from, &len);

		/* eroare la acceptarea conexiunii de la un client */
		if (client < 0)
		{
		  perror ("[server] Eroare la accept().\n");
		  continue;
		}

		  if (nfds < client) /* ajusteaza valoarea maximului */
		    nfds = client;
		    
		/* includem in lista de descriptori activi si acest socket */
		FD_SET (client, &actfds);

		printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n",client, conv_addr (from));
		fflush (stdout);
		}

		if(nfds%2 == 1 && nfds > 3 && clients[nfds] == 0 && clients[nfds - 1] == 0){
			clients[nfds] = clients[nfds - 1] = 1;

			if((child = fork() == 0)){
				player1_fd = nfds - 1;
				player1_turn = 1;
	        	player1_color = 'r';
        		player2_fd = nfds;
        		player2_turn = 0;
        		player2_color = 'y';

        		Init_Board(board);
				bzero(message, 100);
        		strcpy(message, "You are the first player");
        		printf("%d\n", strlen(message));
				write(player1_fd, message, strlen(message));
        		strcpy(message, "You are the second player. Wait for your oponent to make move!");
				write(player2_fd, message, strlen(message));
        		
        		printf("Game started between clients with descriptors %d and %d\n", player1_fd, player2_fd);
        		close(sd);
        		while(1){
        			if(player1_turn == 1){
        				if(player1_fd != sd){
        					bzero(message, 100);
        					strcpy(message, "It's your turn!");
        					write(player1_fd, message, strlen(message));
        					int bytes = Convert_Board(board).size();
							if(bytes && write(player1_fd, Convert_Board(board).c_str(), bytes) < 0){
								perror ("[server] Error write() to client.\n");
								return 0;
							}
        					if(Client_Communication(player1_fd, player1_color) == -1){
        						close(player1_fd);
                                FD_CLR(player1_fd, &actfds);
                                close(player2_fd);
                                FD_CLR(player2_fd, &actfds);
                                clients[player1_fd] = clients[player2_fd] = 0;
								exit(0);
        					}
        					else if(change_turn){
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
        					if(Client_Communication(player2_fd, player2_color) == -1){
        						close(player2_fd);
                                FD_CLR(player2_fd, &actfds);
                                close(player1_fd);
                                FD_CLR(player1_fd, &actfds);
                                clients[player1_fd] = clients[player2_fd] = 0;
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

		/* vedem daca e pregatit vreun socket client pentru a trimite raspunsul */
		// for (fd = 0; fd <= nfds; fd++)	/* parcurgem multimea de descriptori */
		// {
		// /* este un socket de citire pregatit? */
		// if (fd != sd && FD_ISSET (fd, &readfds))
		// {	
		//   if (sayHello(fd))
		// {
		//   printf ("[server] S-a deconectat clientul cu descriptorul %d.\n",fd);
		//   fflush (stdout);
		//   close (fd);		/* inchidem conexiunea cu clientul */
		//   FD_CLR (fd, &actfds);/* scoatem si din multime */
		  
		// }
		// }
		// }			/* for */
	}			
}