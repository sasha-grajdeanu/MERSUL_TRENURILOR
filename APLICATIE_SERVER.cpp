#include<iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include "pugixml.hpp"
#define PORT 2504
using namespace std;

typedef struct thread_data
{
    int id_thread;
    int client;
}thread_data;

pthread_mutex_t MUTEX=PTHREAD_MUTEX_INITIALIZER;

static void *tratare(void*);

void Sistemul_Central(void*);

void mersul_trenurilor(char* locatie);
void deplasari_curente(char* locatie, int location);
int check_parola(char* parola);
int modificare_planificare(char* nr_tren, char* tip_modificare, char* minute);

int main()
{
    int socket_principal;
    struct sockaddr_in Server_Zentral;
    struct sockaddr_in per_client;
    pthread_t MULTIME;
    int i=0;

    if((socket_principal = socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        perror("EROARE LA CREAREA SOCKETULUI[SERVER_ZENTRAL].\n");
        return errno;
    }
    int pornit = 1;
    setsockopt(socket_principal, SOL_SOCKET, SO_REUSEADDR,&pornit, sizeof(pornit));

    bzero(&Server_Zentral, sizeof(Server_Zentral));
    bzero(&per_client, sizeof(per_client));

    Server_Zentral.sin_family=AF_INET;
    Server_Zentral.sin_addr.s_addr = htonl(INADDR_ANY);
    Server_Zentral.sin_port = htons(PORT);

    if(bind(socket_principal, (struct sockaddr *) &Server_Zentral, sizeof(struct sockaddr))==-1)
    {
        perror("EROARE LA BIND [SERVER_ZENTRAL]\n");
        return errno;
    }
    if (listen(socket_principal, 2)==-1)
    {
        perror("EROARE LA LISTEN [SERVER_ZENTRAL]\n");
        return errno;
    }

    while(1)
    {
        printf("SA VINA pe portu %d\n", PORT);
        int clientela;
        thread_data* fir;
        socklen_t lungime = sizeof(per_client);
        if((clientela = accept(socket_principal, (struct sockaddr*) &per_client, &lungime))<0)
        {
            perror ("server | eroare la accept\n");
            continue;
        }
        fir = (struct thread_data*)malloc(sizeof(struct thread_data*));
        fir ->id_thread = i;
        fir ->client = clientela;
        pthread_create(&MULTIME, NULL, &tratare, fir);
        i++;
    }
    return 0;
;}

static void *tratare (void * arg)
{
    struct thread_data fire;
    fire = *((struct thread_data*)arg);
    printf("threadul %d asteapta mesaj\n",fire.id_thread);
    pthread_detach(pthread_self());
    Sistemul_Central((struct thread_data*)arg);
    printf("Inchis thread\n");
    close((intptr_t)arg);
    return NULL;
};

void Sistemul_Central(void *arg)
{
    struct thread_data firisor;
    firisor = *((struct thread_data*)arg);
    int client = firisor.client;
    int terminat_comanda;
    int lungime_msj_primit;
    int lungime_msj_trimis;
    char mesaj_primit[256];
    char mesaj_trimis[8196];
    int comunicare_client_server=1;
    printf("hello\n");
    while(comunicare_client_server == 1)
    {
        bzero(&terminat_comanda, sizeof(int));
        bzero(&lungime_msj_primit, sizeof(int));
        bzero(&lungime_msj_trimis, sizeof(int));
        bzero(mesaj_primit, 256);
        bzero(mesaj_trimis, 8196);
        int citire_lungime = read(client, &lungime_msj_primit, sizeof(int));
        if(citire_lungime==-1)
        {
            perror("EROARE la citire\n");
            break;
        }
        int citire_mesaj_primit = read(client, mesaj_primit, lungime_msj_primit);
        if(citire_mesaj_primit==-1)
        {
            perror("EROARE la citire mesaj\n");
            break;
        }
        if(strcmp(mesaj_primit, "afisare_mersul_trenurilor")==0)
        {
            pthread_mutex_lock(&MUTEX);
            char rezultat_parsare[8196];
            bzero(rezultat_parsare, 8196);
            mersul_trenurilor(rezultat_parsare);
            cout<<rezultat_parsare;
            strcpy(mesaj_trimis, rezultat_parsare);
            lungime_msj_trimis =strlen(mesaj_trimis);
            int trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
            if(trimitere_lungime_msj_trimis==-1)
            {
                perror("EROARE la scriere\n");
                break;
            }
            int trimitere_msj_trimis = write(client, mesaj_trimis, strlen(mesaj_trimis));
            if(trimitere_msj_trimis==-1)
            {
                perror("EROARE la scriere mesaj\n");
                break;
            }
            terminat_comanda = 1;
            int finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
            if(finish_comanda==-1)
            {
                perror("EROARE la scriere mesaj\n");
                break;
            } 
            pthread_mutex_unlock(&MUTEX);           
        }
        else if(strcmp(mesaj_primit, "plecari_ultima_ora")==0)
        {
            pthread_mutex_lock(&MUTEX);
            char rezultat_parsare[8196];
            bzero(rezultat_parsare, 8196);
            deplasari_curente(rezultat_parsare, 1);
            cout<<rezultat_parsare;
            strcpy(mesaj_trimis, rezultat_parsare);
            lungime_msj_trimis =strlen(mesaj_trimis);
            int trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
            if(trimitere_lungime_msj_trimis==-1)
            {
                perror("EROARE la scriere\n");
                break;
            }
            int trimitere_msj_trimis = write(client, mesaj_trimis, strlen(mesaj_trimis));
            if(trimitere_msj_trimis==-1)
            {
                perror("EROARE la scriere mesaj\n");
                break;
            }
            terminat_comanda = 1;
            int finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
            if(finish_comanda==-1)
            {
                perror("EROARE la scriere mesaj\n");
                break;
            }
            pthread_mutex_unlock(&MUTEX); 
        }
        else if(strcmp(mesaj_primit, "sosiri_ultima_ora")==0)
        {
            pthread_mutex_lock(&MUTEX);
            char rezultat_parsare[8196];
            bzero(rezultat_parsare, 8196);
            deplasari_curente(rezultat_parsare, 0);
            cout<<rezultat_parsare;
            strcpy(mesaj_trimis, rezultat_parsare);
            lungime_msj_trimis =strlen(mesaj_trimis);
            int trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
            if(trimitere_lungime_msj_trimis==-1)
            {
                perror("EROARE la scriere\n");
                break;
            }
            int trimitere_msj_trimis = write(client, mesaj_trimis, strlen(mesaj_trimis));
            if(trimitere_msj_trimis==-1)
            {
                perror("EROARE la scriere mesaj\n");
                break;
            }
            terminat_comanda = 1;
            int finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
            if(finish_comanda==-1)
            {
                perror("EROARE la scriere mesaj\n");
                break;
            }
            pthread_mutex_unlock(&MUTEX); 
        }
        else if(strcmp(mesaj_primit, "actualizare")==0)
        {
            strcpy(mesaj_trimis, "Furnizati parola: ");
            lungime_msj_trimis = strlen(mesaj_trimis);
            int trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
            if(trimitere_lungime_msj_trimis==-1)
            {
                perror("EROARE la scriere\n");
                break;
            }
            int trimitere_msj_trimis = write(client, mesaj_trimis, strlen(mesaj_trimis));
            if(trimitere_msj_trimis==-1)
            {
                perror("EROARE la scriere mesaj\n");
                break;
            }
            terminat_comanda = 0;
            int finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
            if(finish_comanda==-1)
            {
                perror("EROARE la scriere mesaj\n");
                break;
            }
            bzero(&terminat_comanda, sizeof(int));
            bzero(&lungime_msj_primit, sizeof(int));
            bzero(&lungime_msj_trimis, sizeof(int));
            bzero(mesaj_primit, 256);
            bzero(mesaj_trimis, 8196);
            int citire_lungime = read(client, &lungime_msj_primit, sizeof(int));
            if(citire_lungime==-1)
            {
                perror("EROARE la citire\n");
                break;
            }
            int citire_mesaj_primit = read(client, mesaj_primit, lungime_msj_primit);
            if(citire_mesaj_primit==-1)
            {
                perror("EROARE la citire mesaj\n");
                break;
            }
            int corect = check_parola(mesaj_primit);
            if(corect==0)
            {
                char raspuns[] = "PAROLA INCORECTA! ACCES INTERZIS.";
                lungime_msj_trimis = strlen(raspuns);
                int trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
                if(trimitere_lungime_msj_trimis==-1)
                {
                    perror("EROARE la scriere\n");
                    break;
                }
                int trimitere_msj_trimis = write(client, raspuns, strlen(raspuns));
                if(trimitere_msj_trimis==-1)
                {
                    perror("EROARE la scriere mesaj\n");
                    break;
                }
                terminat_comanda = 1;
                int finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
                if(finish_comanda==-1)
                {
                    perror("EROARE la scriere mesaj\n");
                    break;
                }
            }
            else
            {
                char nr_tren[10];
                char tip_modificare[10];
                char minute[10];
                strcpy(mesaj_trimis, "Furnizati numarul trenului: ");
                lungime_msj_trimis = strlen(mesaj_trimis);
                trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
                if(trimitere_lungime_msj_trimis==-1)
                {
                    perror("EROARE la scriere\n");
                    break;
                }
                trimitere_msj_trimis = write(client, mesaj_trimis, strlen(mesaj_trimis));
                if(trimitere_msj_trimis==-1)
                {
                    perror("EROARE la scriere mesaj\n");
                    break;
                }
                terminat_comanda = 0;
                finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
                if(finish_comanda==-1)
                {
                    perror("EROARE la scriere mesaj\n");
                    break;
                }
                bzero(&terminat_comanda, sizeof(int));
                bzero(&lungime_msj_primit, sizeof(int));
                bzero(&lungime_msj_trimis, sizeof(int));
                bzero(mesaj_primit, 256);
                bzero(mesaj_trimis, 8196);
                citire_lungime = read(client, &lungime_msj_primit, sizeof(int));
                if(citire_lungime==-1)
                {
                    perror("EROARE la citire\n");
                    break;
                }
                citire_mesaj_primit = read(client, mesaj_primit, lungime_msj_primit);
                if(citire_mesaj_primit==-1)
                {
                    perror("EROARE la citire mesaj\n");
                    break;
                }
                strcpy(nr_tren, mesaj_primit);
                strcpy(mesaj_trimis, "Furnizati tipul modificarii (INT = intarziere, DEV = mai devreme, CCP = conform cu planul) : ");
                lungime_msj_trimis = strlen(mesaj_trimis);
                trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
                if(trimitere_lungime_msj_trimis==-1)
                {
                    perror("EROARE la scriere\n");
                    break;
                }
                trimitere_msj_trimis = write(client, mesaj_trimis, strlen(mesaj_trimis));
                if(trimitere_msj_trimis==-1)
                {
                    perror("EROARE la scriere mesaj\n");
                    break;
                }
                terminat_comanda = 0;
                finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
                if(finish_comanda==-1)
                {
                    perror("EROARE la scriere mesaj\n");
                    break;
                }
                bzero(&terminat_comanda, sizeof(int));
                bzero(&lungime_msj_primit, sizeof(int));
                bzero(&lungime_msj_trimis, sizeof(int));
                bzero(mesaj_primit, 256);
                bzero(mesaj_trimis, 8196);
                citire_lungime = read(client, &lungime_msj_primit, sizeof(int));
                if(citire_lungime==-1)
                {
                    perror("EROARE la citire\n");
                    break;
                }
                citire_mesaj_primit = read(client, mesaj_primit, lungime_msj_primit);
                if(citire_mesaj_primit==-1)
                {
                    perror("EROARE la citire mesaj\n");
                    break;
                }
                strcpy(tip_modificare, mesaj_primit);
                strcpy(mesaj_trimis, "Furnizati timpul (in cazul in care trenul respecta planificarea, scrieti 0): ");
                lungime_msj_trimis = strlen(mesaj_trimis);
                trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
                if(trimitere_lungime_msj_trimis==-1)
                {
                    perror("EROARE la scriere\n");
                    break;
                }
                trimitere_msj_trimis = write(client, mesaj_trimis, strlen(mesaj_trimis));
                if(trimitere_msj_trimis==-1)
                {
                    perror("EROARE la scriere mesaj\n");
                    break;
                }
                terminat_comanda = 0;
                finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
                if(finish_comanda==-1)
                {
                    perror("EROARE la scriere mesaj\n");
                    break;
                }
                bzero(&terminat_comanda, sizeof(int));
                bzero(&lungime_msj_primit, sizeof(int));
                bzero(&lungime_msj_trimis, sizeof(int));
                bzero(mesaj_primit, 256);
                bzero(mesaj_trimis, 8196);
                citire_lungime = read(client, &lungime_msj_primit, sizeof(int));
                if(citire_lungime==-1)
                {
                    perror("EROARE la citire\n");
                    break;
                }
                citire_mesaj_primit = read(client, mesaj_primit, lungime_msj_primit);
                if(citire_mesaj_primit==-1)
                {
                    perror("EROARE la citire mesaj\n");
                    break;
                }
                strcpy(minute, mesaj_primit);
                pthread_mutex_lock(&MUTEX);
                int modification = modificare_planificare(nr_tren, tip_modificare, minute);
                if(modification == -1)//nr tren gresit/neexistent
                {
                    strcpy(mesaj_trimis, "Nu exista tren cu acest numar\n.");
                    lungime_msj_trimis = strlen(mesaj_trimis);
                    trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
                    if(trimitere_lungime_msj_trimis==-1)
                    {
                        perror("EROARE la scriere\n");
                        break;
                    }
                    trimitere_msj_trimis = write(client, mesaj_trimis, strlen(mesaj_trimis));
                    if(trimitere_msj_trimis==-1)
                    {
                        perror("EROARE la scriere mesaj\n");
                        break;
                    }
                    terminat_comanda = 1;
                    finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
                    if(finish_comanda==-1)
                    {
                        perror("EROARE la scriere mesaj\n");
                        break;
                    }
                }
                else if(modification == -2)//parametru gresit la tip_modificare
                {
                    strcpy(mesaj_trimis, "Parametru gresit la tip modificare.\n");
                    lungime_msj_trimis = strlen(mesaj_trimis);
                    trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
                    if(trimitere_lungime_msj_trimis==-1)
                    {
                        perror("EROARE la scriere\n");
                        break;
                    }
                    trimitere_msj_trimis = write(client, mesaj_trimis, strlen(mesaj_trimis));
                    if(trimitere_msj_trimis==-1)
                    {
                        perror("EROARE la scriere mesaj\n");
                        break;
                    }
                    terminat_comanda = 1;
                    finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
                    if(finish_comanda==-1)
                    {
                        perror("EROARE la scriere mesaj\n");
                        break;
                    }
                }
                else if(modification == -3)//parametru gresit la minute
                {
                    strcpy(mesaj_trimis, "Parametru gresit la minute.\n");
                    lungime_msj_trimis = strlen(mesaj_trimis);
                    trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
                    if(trimitere_lungime_msj_trimis==-1)
                    {
                        perror("EROARE la scriere\n");
                        break;
                    }
                    trimitere_msj_trimis = write(client, mesaj_trimis, strlen(mesaj_trimis));
                    if(trimitere_msj_trimis==-1)
                    {
                        perror("EROARE la scriere mesaj\n");
                        break;
                    }
                    terminat_comanda = 1;
                    finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
                    if(finish_comanda==-1)
                    {
                        perror("EROARE la scriere mesaj\n");
                        break;
                    }
                }
                else//reusit
                {
                    strcpy(mesaj_trimis, "Modificare reusita.\n");
                    lungime_msj_trimis = strlen(mesaj_trimis);
                    trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
                    if(trimitere_lungime_msj_trimis==-1)
                    {
                        perror("EROARE la scriere\n");
                        break;
                    }
                    trimitere_msj_trimis = write(client, mesaj_trimis, strlen(mesaj_trimis));
                    if(trimitere_msj_trimis==-1)
                    {
                        perror("EROARE la scriere mesaj\n");
                        break;
                    }
                    terminat_comanda = 1;
                    finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
                    if(finish_comanda==-1)
                    {
                        perror("EROARE la scriere mesaj\n");
                        break;
                    }
                }
                pthread_mutex_unlock(&MUTEX);
            }
        }
        else if(strcmp(mesaj_primit, "end_connex")==0)
        {
            char raspuns[] = "END_CONNEX\n";
            lungime_msj_trimis = strlen(raspuns);
            int trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
            if(trimitere_lungime_msj_trimis==-1)
            {
                perror("EROARE la scriere\n");
                break;
            }
            int trimitere_msj_trimis = write(client, raspuns, strlen(raspuns));
            if(trimitere_msj_trimis==-1)
            {
                perror("EROARE la scriere mesaj\n");
                break;
            }
            terminat_comanda = 1;
            int finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
            if(finish_comanda==-1)
            {
                perror("EROARE la scriere mesaj\n");
                break;
            }
            comunicare_client_server = 0;
        }
        else
        {
            char raspuns[] = "comanda_gresita\n";
            lungime_msj_trimis = strlen(raspuns);
            int trimitere_lungime_msj_trimis = write(client, &lungime_msj_trimis, sizeof(int));
            if(trimitere_lungime_msj_trimis==-1)
            {
                perror("EROARE la scriere\n");
                break;
            }
            int trimitere_msj_trimis = write(client, raspuns, strlen(raspuns));
            if(trimitere_msj_trimis==-1)
            {
                perror("EROARE la scriere mesaj\n");
                break;
            }
            terminat_comanda = 1;
            int finish_comanda = write(client, &terminat_comanda, sizeof(terminat_comanda));
            if(finish_comanda==-1)
            {
                perror("EROARE la scriere mesaj\n");
                break;
            }
        }
    }
    cout<<"Inchis conexiune"<<endl;
    close(client);
}

void mersul_trenurilor(char* locatie)
{
    char rezultat[8196];
    bzero(rezultat, 8196);
    pugi::xml_document tabela;
    pugi::xml_parse_result actual = tabela.load_file("Planificare_tren.xml");
    cout<<"HELI"<<endl;
    for(int i = 0; i<26; i++)
    {
        strcat(rezultat, "=");
    }
    strcat(rezultat, "PLECARI");
    for(int i = 0; i<27; i++)
    {
        strcat(rezultat, "=");
    }
    strcat(rezultat, "\n");
    strcat(rezultat, "NUMAR");
    for(int i = 0; i<(8-5); i++)
    {
        strcat(rezultat, " ");
    }
    strcat(rezultat, "DIRECTIE");
    for(int i = 0; i<(20-8); i++)
    {
        strcat(rezultat, " ");
    }
    strcat(rezultat, "ORA");
    for(int i = 0; i<(8-3); i++)
    {
        strcat(rezultat, " ");
    }
    strcat(rezultat, "MENTIUNI");
    for(int i = 0; i<(24-8); i++)
    {
        strcat(rezultat, " ");
    }
    strcat(rezultat, "\n");
    for(int i = 0; i<60; i++)
    {
        strcat(rezultat, "*");
    }
    strcat(rezultat, "\n");
    for(pugi::xml_node tren = tabela.child("TABELA").child("PLECARI").first_child(); tren; tren = tren.next_sibling())
    {
        char numar[10];
        strcpy(numar, tren.child("NUMAR").attribute("nr").value());
        strcat(rezultat, numar);
        int lungime = strlen(numar);
        for(int i=0; i<(8-lungime); i++)
        {
            strcat(rezultat, " ");
        }
        char directie[50];
        strcpy(directie, tren.child("DESTINATIE").attribute("dest").value());
        lungime = strlen(directie);
        strcat(rezultat, directie);
        for(int i=0; i<(20-lungime); i++)
        {
            strcat(rezultat, " ");
        }
        char ora[10];
        strcpy(ora, tren.child("ORA").attribute("h").value());
        lungime = strlen(ora);
        strcat(rezultat, ora);
        for(int i=0; i<(8-lungime); i++)
        {
            strcat(rezultat, " ");
        }
        char mentiuni[50];
        strcpy(mentiuni, tren.child("MENTIUNI").attribute("ment").value());
        lungime = strlen(mentiuni);
        strcat(rezultat, mentiuni);
        for(int i=0; i<(24-lungime); i++)
        {
            strcat(rezultat, " ");
        }
        strcat(rezultat, "\n");
    }
    for(int i = 0; i<60; i++)
    {
        strcat(rezultat, "-");
    }
    strcat(rezultat, "\n");
    for(int i = 0; i<26; i++)
    {
        strcat(rezultat, "=");
    }
    strcat(rezultat, "=SOSIRI");
    for(int i = 0; i<27; i++)
    {
        strcat(rezultat, "=");
    }
    strcat(rezultat, "\n");
    strcat(rezultat, "NUMAR");
    for(int i = 0; i<(8-5); i++)
    {
        strcat(rezultat, " ");
    }
    strcat(rezultat, "DIRECTIE");
    for(int i = 0; i<(20-8); i++)
    {
        strcat(rezultat, " ");
    }
    strcat(rezultat, "ORA");
    for(int i = 0; i<(8-3); i++)
    {
        strcat(rezultat, " ");
    }
    strcat(rezultat, "MENTIUNI");
    for(int i = 0; i<(24-8); i++)
    {
        strcat(rezultat, " ");
    }
    strcat(rezultat, "\n");
    for(int i = 0; i<60; i++)
    {
        strcat(rezultat, "*");
    }
    strcat(rezultat, "\n");
    for(pugi::xml_node tren = tabela.child("TABELA").child("SOSIRI").first_child(); tren; tren = tren.next_sibling())
    {
        char numar[10];
        strcpy(numar, tren.child("NUMAR").attribute("nr").value());
        strcat(rezultat, numar);
        int lungime = strlen(numar);
        for(int i=0; i<(8-lungime); i++)
        {
            strcat(rezultat, " ");
        }
        char directie[50];
        strcpy(directie, tren.child("DESTINATIE").attribute("dest").value());
        lungime = strlen(directie);
        strcat(rezultat, directie);
        for(int i=0; i<(20-lungime); i++)
        {
            strcat(rezultat, " ");
        }
        char ora[10];
        strcpy(ora, tren.child("ORA").attribute("h").value());
        lungime = strlen(ora);
        strcat(rezultat, ora);
        for(int i=0; i<(8-lungime); i++)
        {
            strcat(rezultat, " ");
        }
        char mentiuni[50];
        strcpy(mentiuni, tren.child("MENTIUNI").attribute("ment").value());
        lungime = strlen(mentiuni);
        strcat(rezultat, mentiuni);
        for(int i=0; i<(24-lungime); i++)
        {
            strcat(rezultat, " ");
        }
        strcat(rezultat, "\n");
    }
    strcpy(locatie, rezultat);
}

void deplasari_curente(char* locatie, int location)
{
    char rezultat[8196];
    bzero(rezultat, 8196);
    time_t ceas;
    struct tm* information;
    char ora_exacta[8];
    char ora_viitoare[8];
    time(&ceas);
    information = localtime(&ceas);
    strftime(ora_exacta, 8, "%H:%M", information);
    cout<<ora_exacta<<endl;
    information->tm_hour=(information->tm_hour+1)%24;
    strftime(ora_viitoare, 8, "%H:%M", information);
    pugi::xml_document tabela;
    pugi::xml_parse_result actual = tabela.load_file("Planificare_tren.xml");
    if(location == 1) //plecari
    {
        cout<<"HELI"<<endl;
        for(int i = 0; i<21; i++)
        {
            strcat(rezultat, "=");
        }
        strcat(rezultat, "PLECARI=ULTIMA=ORA");
        for(int i = 0; i<21; i++)
        {
            strcat(rezultat, "=");
        }
        strcat(rezultat, "\n");
        strcat(rezultat, "NUMAR");
        for(int i = 0; i<(8-5); i++)
        {
            strcat(rezultat, " ");
        }
        strcat(rezultat, "DIRECTIE");
        for(int i = 0; i<(20-8); i++)
        {
            strcat(rezultat, " ");
        }
        strcat(rezultat, "ORA");
        for(int i = 0; i<(8-3); i++)
        {
            strcat(rezultat, " ");
        }
        strcat(rezultat, "MENTIUNI");
        for(int i = 0; i<(24-8); i++)
        {
            strcat(rezultat, " ");
        }
        strcat(rezultat, "\n");
        for(int i = 0; i<60; i++)
        {
            strcat(rezultat, "*");
        }
        strcat(rezultat, "\n");
        for(pugi::xml_node tren = tabela.child("TABELA").child("PLECARI").first_child(); tren; tren = tren.next_sibling())
        {
            if(strcmp(tren.child("ORA").attribute("h").value(), ora_exacta)>=0 && strcmp(tren.child("ORA").attribute("h").value(), ora_viitoare)<=0)
            {
                char numar[10];
                strcpy(numar, tren.child("NUMAR").attribute("nr").value());
                strcat(rezultat, numar);
                int lungime = strlen(numar);
                for(int i=0; i<(8-lungime); i++)
                {
                    strcat(rezultat, " ");
                }
                char directie[50];
                strcpy(directie, tren.child("DESTINATIE").attribute("dest").value());
                lungime = strlen(directie);
                strcat(rezultat, directie);
                for(int i=0; i<(20-lungime); i++)
                {
                    strcat(rezultat, " ");
                }
                char ora[10];
                strcpy(ora, tren.child("ORA").attribute("h").value());
                lungime = strlen(ora);
                strcat(rezultat, ora);
                for(int i=0; i<(8-lungime); i++)
                {
                    strcat(rezultat, " ");
                }
                char mentiuni[50];
                strcpy(mentiuni, tren.child("MENTIUNI").attribute("ment").value());
                lungime = strlen(mentiuni);
                strcat(rezultat, mentiuni);
                for(int i=0; i<(24-lungime); i++)
                {
                    strcat(rezultat, " ");
                }
                strcat(rezultat, "\n");
            }
        }
        strcpy(locatie, rezultat);
    }
    else//sosiri
    {
        cout<<"HELI"<<endl;
        for(int i = 0; i<21; i++)
        {
            strcat(rezultat, "=");
        }
        strcat(rezultat, "SOSIRI=ULTIMA=ORA=");
        for(int i = 0; i<21; i++)
        {
            strcat(rezultat, "=");
        }
        strcat(rezultat, "\n");
        strcat(rezultat, "NUMAR");
        for(int i = 0; i<(8-5); i++)
        {
            strcat(rezultat, " ");
        }
        strcat(rezultat, "DIRECTIE");
        for(int i = 0; i<(20-8); i++)
        {
            strcat(rezultat, " ");
        }
        strcat(rezultat, "ORA");
        for(int i = 0; i<(8-3); i++)
        {
            strcat(rezultat, " ");
        }
        strcat(rezultat, "MENTIUNI");
        for(int i = 0; i<(24-8); i++)
        {
            strcat(rezultat, " ");
        }
        strcat(rezultat, "\n");
        for(int i = 0; i<60; i++)
        {
            strcat(rezultat, "*");
        }
        strcat(rezultat, "\n");
        for(pugi::xml_node tren = tabela.child("TABELA").child("SOSIRI").first_child(); tren; tren = tren.next_sibling())
        {
            if(strcmp(tren.child("ORA").attribute("h").value(), ora_exacta)>=0 && strcmp(tren.child("ORA").attribute("h").value(), ora_viitoare)<=0)
            {
                char numar[10];
                strcpy(numar, tren.child("NUMAR").attribute("nr").value());
                strcat(rezultat, numar);
                int lungime = strlen(numar);
                for(int i=0; i<(8-lungime); i++)
                {
                    strcat(rezultat, " ");
                }
                char directie[50];
                strcpy(directie, tren.child("DESTINATIE").attribute("dest").value());
                lungime = strlen(directie);
                strcat(rezultat, directie);
                for(int i=0; i<(20-lungime); i++)
                {
                    strcat(rezultat, " ");
                }
                char ora[10];
                strcpy(ora, tren.child("ORA").attribute("h").value());
                lungime = strlen(ora);
                strcat(rezultat, ora);
                for(int i=0; i<(8-lungime); i++)
                {
                    strcat(rezultat, " ");
                }
                char mentiuni[50];
                strcpy(mentiuni, tren.child("MENTIUNI").attribute("ment").value());
                lungime = strlen(mentiuni);
                strcat(rezultat, mentiuni);
                for(int i=0; i<(24-lungime); i++)
                {
                    strcat(rezultat, " ");
                }
                strcat(rezultat, "\n");
            }
        }
        strcpy(locatie, rezultat);
    }
}

int check_parola(char* parola)
{
    if(strcmp(parola,"Glasul_rotilor_de_tren")==0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int modificare_planificare(char* nr_tren, char* tip_modificare, char* minute)
{
    int gasit=0;
    pugi::xml_document tabela;
    pugi::xml_parse_result actual = tabela.load_file("Planificare_tren.xml");
    if(strcmp(tip_modificare, "INT")==0)//se solicita intarzierea sosirii unui tren
    {
        if(strcmp(minute, "0")>0 || strcmp(minute, "1440")<=0)
        {
            for(pugi::xml_node tren = tabela.child("TABELA").child("PLECARI").first_child(); tren; tren = tren.next_sibling())
            {
                if(strcmp(nr_tren, tren.child("NUMAR").attribute("nr").value())==0)
                {
                    gasit++;
                    char nou[100];
                    strcpy(nou, "Intarziere: ");
                    strcat(nou, minute);
                    strcat(nou, " minute.");
                    tren.child("MENTIUNI").attribute("ment").set_value(nou);
                    tabela.save_file("Planificare_tren.xml");
                    break;
                }
            }
            for(pugi::xml_node tren = tabela.child("TABELA").child("SOSIRI").first_child(); tren; tren = tren.next_sibling())
            {
                if(strcmp(nr_tren, tren.child("NUMAR").attribute("nr").value())==0)
                {
                    gasit++;
                    char nou[100];
                    strcpy(nou, "Intarziere: ");
                    strcat(nou, minute);
                    strcat(nou, " minute.");
                    tren.child("MENTIUNI").attribute("ment").set_value(nou);
                    tabela.save_file("Planificare_tren.xml");
                    break;
                }
            }
            if(gasit > 0)
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -3;
        }
    }
    else if(strcmp(tip_modificare, "DEV")==0)//se solicita ajungerea mai devreme a unui tren
    {
        if(strcmp(minute, "0")>0 || strcmp(minute, "1440")<=0)
        {
            for(pugi::xml_node tren = tabela.child("TABELA").child("PLECARI").first_child(); tren; tren = tren.next_sibling())
            {
                if(strcmp(nr_tren, tren.child("NUMAR").attribute("nr").value())==0)
                {
                    gasit++;
                    char nou[100];
                    strcpy(nou, "Conform cu planul");
                    tren.child("MENTIUNI").attribute("ment").set_value(nou);
                    tabela.save_file("Planificare_tren.xml");
                    break;
                }
            }
            for(pugi::xml_node tren = tabela.child("TABELA").child("SOSIRI").first_child(); tren; tren = tren.next_sibling())
            {
                if(strcmp(nr_tren, tren.child("NUMAR").attribute("nr").value())==0)
                {
                    gasit++;
                    char nou[100];
                    strcpy(nou, "Mai devreme cu ");
                    strcat(nou, minute);
                    strcat(nou, " minute.");
                    tren.child("MENTIUNI").attribute("ment").set_value(nou);
                    tabela.save_file("Planificare_tren.xml");
                    break;
                }
            }
            if(gasit > 0)
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -3;
        }
    }
    else if(strcmp(tip_modificare, "CCP")==0)//se solicita respectarea planului
    {
        for(pugi::xml_node tren = tabela.child("TABELA").child("PLECARI").first_child(); tren; tren = tren.next_sibling())
        {
            if(strcmp(nr_tren, tren.child("NUMAR").attribute("nr").value())==0)
            {
                gasit++;
                char nou[100];
                strcpy(nou, "Conform cu planul");
                tren.child("MENTIUNI").attribute("ment").set_value(nou);
                tabela.save_file("Planificare_tren.xml");
                break;
            }
        }
        for(pugi::xml_node tren = tabela.child("TABELA").child("SOSIRI").first_child(); tren; tren = tren.next_sibling())
        {
            if(strcmp(nr_tren, tren.child("NUMAR").attribute("nr").value())==0)
            {
                gasit++;
                char nou[100];
                strcpy(nou, "Conform cu planul");
                tren.child("MENTIUNI").attribute("ment").set_value(nou);
                tabela.save_file("Planificare_tren.xml");
                break;
            }
        }
        if(gasit > 0)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -2;
    }
}