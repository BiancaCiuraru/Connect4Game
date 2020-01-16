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

#define PORT 2728
using namespace std;

extern int errno;

char A[7][7];
int vizA[4] = {0};
int vizB[4] = {0};

void CreateTable(char A[7][7]) {
    int i, j;
    for(i = 0; i < 6; i++)
        for(j = 0; j < 7; j++) A[i][j] = ' '; // umplu matricea cu ' '

    for(i = 1; i <= 7; i++) A[7][i] = '1' + i - 1; // bordare pe coloana
}

struct Player {
    int round;
    char color;
    int fd;
};


string Convert(char A[7][7]) {
    int i, j;
    string s = "";
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 7; j++) {
            s += A[i][j];
        }
    }
    return s;
}


void updateTable(char player_color, char move){
    for(int i = 5; i >= 0; i--)
        if(A[i][move] == ' ')
            A[i][move] = player_color;
}

char mesaj[250];
int validMove(int fd, int coloana) { 
    if(coloana < 1 || coloana > 7) // jucatorul a introdus o coloana inexistenta
    {
        return 2;
    }
    if(A[0][coloana] != 0) // coloana introdusa este plina
    {
        return 3;
    }

    return 1;
}

void Copy(char B[7][7], char A[7][7]) {
    int i, j;
    for(i = 0; i < 6; i++)
        for(j = 0; j < 7; j++)
            B[i][j] = A[i][j];
}

int connect() {
    for(int i=0; i<6;i++)
        for(int j=0; j<7;j++){
            int nr = 1, i1 = i, j1 = j-1;
            while(A[i][j] == A[i1][j1] && nr < 4 && j1 >= 0)
            {
                j1--; nr++;
            }
            j1 = j + 1; 
            while(A[i][j] == A[i1][j1] && nr < 4 && j1 < 7)
            {
                j1++; nr++;
            }
            if(nr >= 4) return 1; //castig pe orizontala

            nr = 1, i1 = i - 1, j1 = j - 1;
            while(A[i][j] == A[i1][j1] && nr < 4 && i1 >=0 && j1 >= 0)
            {
                i1--; j1--; nr++;
            }
            i1 = i + 1; j1 = j + 1; 
            while(A[i][j] == A[i1][j1] && nr < 4 && i1 < 6 && j1 <7)
            {
                i1++; j1++; nr++;
            }
            if(nr >= 4) return 1; //castig pe o diagonala

            nr = 1, i1 = i - 1, j1 = j + 1;
            while(A[i][j] == A[i1][j1] && nr < 4 && i1 >= 0 && j1 < 7)
            {
                i1--; j1++; nr++;
            }
            i1 = i + 1; j1 = j - 1; 
            while(A[i][j] == A[i1][j1] && nr < 4 && i1 <6 && j1 >= 0)
            {
                i1++; j1--; nr++;
            }
            if(nr >= 4) return 1; //castig pe cealalta diagonala

        /*4*/nr = 1, i1 = i - 1, j1 = j;
            while(A[i][j] == A[i1][j1] && nr < 4 && i1 >= 0)
            {
                i1--; nr++;
            }
            i1 = i + 1;
            while(A[i][j] == A[i1][j1] && nr < 4 && i1 < 6)
            {
                i1++; nr++;
            }
            if(nr >= 4) return 1; //castig pe verticala

        }
   
    return 0;// inca nu a castigat nimeni
}

int Message(int fd, int &verify, char move)
{
    string s;
    char buffer[100];
    int bytes;
    char msg[100], save;
    char msgrasp[100]=" ";
    
    bytes = read (fd, msg, sizeof (buffer));
    if (bytes < 0)
    {
        perror ("Eroare la read() de la client.\n");
        return 0;
    }

    strcpy(msgrasp, msg);
    int i, nr = 0;
    cout << msgrasp<< "mutare";

    if(strcmp(msg, "surrender\n") == 0) return -1;
    else if(validMove(fd, stoi(msgrasp)) == 2){
        strcpy(msg, "Coloana invalida. Nu exista");
        write(fd, msg, strlen(msg));
        return -1;
    }
     else if(validMove(fd, stoi(msgrasp)) == 3){
        strcpy(msg, "coloana plina");
        write(fd, msg, strlen(msg));
        return -1;
    }
     else if(validMove(fd, stoi(msgrasp)) == 1){
        updateTable(move,stoi(msgrasp));
        s = Convert(A);
        bytes = s.size();
        if(connect())
        {
            strcpy(msg, "ai castigat");
            write(fd, msg, strlen(msg));
            return -1;
        }   
    }
        cout << "mutare pozitia " << msgrasp;
        cout << "Se asteapta mutarea celuilalt jucator!" << endl;
        
        if(fd % 2 == 0){
            if (bytes && write (fd + 1, s.c_str(), bytes) < 0)
            {
                perror ("[server] Eroare la write() catre client.\n");
                return 0;
            }
            
            if (strlen(msg) && write(fd, msg, strlen(msg)) < 0){
                perror ("[server] Eroare la write() catre client.\n");
                return 0;
            }
        }
        else if(fd % 2 == 1){
            if (bytes && write (fd - 1, s.c_str(), bytes) < 0)
            {
                perror ("[server] Eroare la write() catre client.\n");
                return 0;
            }
            
            if (strlen(msg) && write(fd, msg, strlen(msg)) < 0){
                perror ("[server] Eroare la write() catre client.\n");
                return 0;
            }
        }
        verify = 1;
        return bytes;

}


char * conv_addr (struct sockaddr_in address)
{
    static char str[25];
    char port[7];

    strcpy (str, inet_ntoa (address.sin_addr));
    bzero (port, 7);
    sprintf (port, ":%d", ntohs (address.sin_port));
    strcat (str, port);
    return (str);
}

int main ()
{
    struct sockaddr_in server;
    struct sockaddr_in from;
    fd_set readfds;
    fd_set actfds;
    struct timeval tv;
    int sd, client;
    int optval=1;
    int fd;
    int nfds;
    pid_t childpid;
    unsigned int len;
    int v[250] = {0};

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
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

        if(nfds % 2 == 1 && nfds > 3 && !v[nfds - 1] && !v[nfds]) {
            //cout << a.fd << " " << b.fd << endl;
            v[nfds - 1] = 1;
            v[nfds] = 1;
            if((childpid = fork()) == 0) {
                CreateTable(A);
                //PrintTable(A);
                cout << "Jocul a inceput!" << endl;
                Player a;
                Player b;
                a.round = 1;
                a.color = 'r';
                a.fd = nfds - 1;
                b.round = 0;
                b.color = 'y';
                b.fd = nfds;
                close(sd);
                int ft = 0;
                while(1) {
                    if(a.round == 1) {
                        fd = a.fd;
                        int verify = 0;
                        if (fd != sd)
                        {   
                            if(!ft){
                                string s;
                                s = Convert(A);
                                int bytes = s.size();
                                if (bytes && write (fd, s.c_str(), bytes) < 0)
                                {
                                     perror ("[server] Eroare la write() catre client.\n");
                                     return 0;
                                }
                                ft = 1;                                                 
                            }
                            if(Message(fd, verify, a.color) == -1) {
                                close (fd);
                                FD_CLR (fd, &actfds);
                                close(fd + 1);
                                FD_CLR(fd + 1, &actfds);
                                v[a.fd] = 0;
                                v[b.fd] = 0;
                                exit(0);
                                //cout << "Jocul a luat sfarsit! Jucatorul B a castigat!" << endl;
                            }
                            else if(verify == 1) {
                                a.round = 0;
                                b.round = 1;
                                verify = 0;
                            }
                        }
                    }
                    if(b.round == 1) {
                        fd = b.fd;
                        int verify = 0;
                        if (fd != sd)
                        {   
                            if(Message(fd, verify, b.color) == -1) {
                                close (fd);
                                FD_CLR (fd, &actfds);
                                close(fd - 1);
                                FD_CLR(fd - 1, &actfds);
                                v[a.fd] = 0;
                                v[b.fd] = 0;
                                exit(0);
                                //cout << "Jocul a luat sfarsit! Jucatorul A a castigat!" << endl;
                            }
                            else if(verify == 1) {
                                b.round = 0;
                                a.round = 1;
                                verify = 0;
                            }
                        }
                    }
                }
            }
        }
    }
}
