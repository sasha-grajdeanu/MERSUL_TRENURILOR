#include<iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#define DONE 1
#define RUNNING 0
#define PORT "2504"
#define ADRESA "127.0.0.1"
using namespace std;

int main()
{
    int descriptor_socket;
    int lungime_comanda;
    int end_connex=0;
    struct sockaddr_in server_central;
    char stocare_mesaj_client[60];  
    char mesaj_primit[8192];
    int rezolvare_comanda=RUNNING;
    int port=atoi(PORT);
    descriptor_socket=socket(AF_INET, SOCK_STREAM,0);
    if(descriptor_socket==-1)
    {
        perror("Eroare la realizarea socketului. \n");
        return errno;
    }

    server_central.sin_family=AF_INET;
    server_central.sin_addr.s_addr=inet_addr(ADRESA);
    server_central.sin_port=htons(port);

    if(connect(descriptor_socket, (struct sockaddr*) &server_central, sizeof(struct sockaddr))==-1)
    {
        perror("Eroare la relizarea conexiunii din partea clientului \n");
        return errno;
    }
    cout<<"==================MERSUL====TRENURILOR======================"<<endl;
    while(1)
    {
        rezolvare_comanda=RUNNING;
        bzero(stocare_mesaj_client, 60);
        bzero(mesaj_primit, 8192);

        cout<<"Introduceti comanda: ";
        cin>>stocare_mesaj_client;
        cout<<endl;

        lungime_comanda=strlen(stocare_mesaj_client);
        int trimite_lungime_mesaj=write(descriptor_socket, &lungime_comanda, sizeof(int)); 
        //trimit lungimea comenzii pentru ca serverul sa stie cat sa citeasca

        if(trimite_lungime_mesaj<=0)
        {
            perror("Eroare la transmiterea lungimii mesajului \n");
            return errno;
        }

        int scriere_1=write(descriptor_socket, stocare_mesaj_client, strlen(stocare_mesaj_client));
        if(scriere_1<=0)
        {
            perror("Eroare la transmiterea comenzii \n");
            return errno;
        }
        //am trimis comanda
        //pana aici doar am trimis serverului comanda
        while(rezolvare_comanda==RUNNING)
        {
            int lungime_mesaj_receptionat;
            int citire_lungime=read(descriptor_socket, &lungime_mesaj_receptionat, sizeof(int)); 
            //citesc lungimea comenzii pentru ca aplicatia client sa stie cat sa citeasca
            int citire_raspunsuri=read(descriptor_socket,mesaj_primit, lungime_mesaj_receptionat);
            //citesc mesajul transmis
            int decizie;
            int finish_command=read(descriptor_socket, &decizie, sizeof(int));
            //verific daca nu cumva serverul mai are nevoie de informatii(clientul a apelat functia actualizare)
            if(decizie==0)//aici consideram ca userul a apelat comanda actualizare
            {
                bzero(stocare_mesaj_client, sizeof(stocare_mesaj_client));//resetez pentru a stoca alte date solicitate de server

                cout<<mesaj_primit; //afisez mesajul trimis de server
                cin>>stocare_mesaj_client; //furnizez informatia ceruta
                cout<<endl;

                bzero(mesaj_primit, sizeof(mesaj_primit)); //resetez pentru a stoca mai tarziu ce mi-a transmis serverul
                
                lungime_comanda=strlen(stocare_mesaj_client);
                int trimite_lungime_mesaj_1=write(descriptor_socket, &lungime_comanda, sizeof(int)); 
                if(trimite_lungime_mesaj_1<=0)
                {
                    perror("Eroare la transmiterea lungimii mesajului in while 2 \n");
                    return errno;
                }
                //din nou trimit lungimea

                int scriere_2=write(descriptor_socket, stocare_mesaj_client, strlen(stocare_mesaj_client));
                if(scriere_2<=0)
                {
                    perror("Eroare la transmiterea comenzii in while 2 \n");
                    return errno;
                }
                //trimit informatia ceruta

            }
            else//aici consideram ca userul a solicitat informatii referitoare la circulatia trenurilor
            {
                cout<<mesaj_primit;
                //afisez rezultatul
                if(strcmp(mesaj_primit, "END_CONNEX\n")==0)
                {
                    end_connex=1;
                }
                //clientu a transmis serverului close
                bzero(stocare_mesaj_client, sizeof(stocare_mesaj_client));
                bzero(mesaj_primit, sizeof(mesaj_primit)); 
                //resetam pentru comenzile viitoare

                rezolvare_comanda=DONE;
                //ca sa iesim din bucla rezolvare_comanda==RUNNING
            }
        }
        for(int i=0; i<60; i++)
        {
            cout<<"=";
        }
        cout<<endl;
        if(end_connex==1)
        {
            break;
        }
    }
    close(descriptor_socket);
    return 0;
}

