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

/* codul de eroare returnat de anumite apeluri */
extern int errno;

using namespace std;

/* portul de conectare la server*/
int port;
void Convert(char a[7][7], string s) {
    int i, j, k = 0;
    for(i = 0; i < 6; i++) {
        for(j = 0; j < 7; j++)
            a[i][j] = s[k++];
    }
}


void Print(char A[7][7]){
    int i, j;
    
    for(i = 0; i < 7; i++) A[7][i] = '1' + i - 1; 

    for(i = 0; i < 6; i++) {
        for(j = 0; j < 7; j++)
            cout << "|" << A[i][j];
        cout << "|" << endl;
    }
    cout << "================" << endl;

    for(j = 1; j <= 7; j++) cout << "|" << j;
    cout << "|"<<endl;
}


int main (int argc, char *argv[])
{
  int sd;           // descriptorul de socket
  struct sockaddr_in server;    // structura folosita pentru conectare 
  char msg[100];        // mesajul trimis

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[client] Eroare la socket().\n");
      return errno;
    }
  

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

    char a[7][7];
    int x = 0;
    string s;
    s.resize(100);
    if (read (sd, &s[0], s.size()) < 0)
    {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
    }
    Convert(a, s);
    Print(a);
    cout << endl;

    while(1) {

        bzero (msg, 100);
        printf ("[client]Introduceti mutarea dorita: ");
        fflush (stdout);
        read (0, msg, 100);

        if (write (sd, msg, 100) <= 0)
        {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
        }
        
        if(strcmp(msg, "surrender\n") == 0) break;


        if (read (sd, msg, 100) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }
        cout << msg << endl;
    
    if(!strstr(msg, "invalid")){
        s.resize(100);
        if (read (sd, &s[0], s.size()) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }
        Convert(a, s);
        Print(a);
        cout << endl;
        }
    }
    close (sd);
}
