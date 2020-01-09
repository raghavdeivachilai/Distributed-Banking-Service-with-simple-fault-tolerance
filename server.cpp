/*Raghav Deivachilai
ZG70451*/
/*This is a backend server which will accept the connections from front end server and process transactions*/

#include <iostream>
#include <fcntl.h>
#include <thread>
#include <arpa/inet.h>
#include <sstream>
#include <cstring>
#include <fstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <map>

using namespace std;
int number = 100;

map<int, float> records;

pthread_mutex_t mutex;

void *Backend_server(void *sockdesc)
{
    int client_sock = *(int *)sockdesc;
    while (true)
    {

        char message1[256];
        bzero(message1, 256);
        int receive1 = read(client_sock, message1, sizeof(message1));
        cout << endl;
        cout << "Coordinator Message:" << message1 << endl;
        time_t timer;
        srand((unsigned)time(&timer));
        int abortval = (rand() % 2)-1;
        cout<<endl;
        cout << "Random Number Generated to show Abort:"<<abortval<<endl;;
        if (abortval == 0)
        {
            cout<<endl;
            cout<<"Server Message: ABORT"<<endl;
            int s1 = send(client_sock, "ABORT", 256, 0);
        }

        if (receive1 != 0 && abortval!=0) 
        {
            cout<<endl;
            cout<<"Server Message: READY"<<endl;
            int s1 = send(client_sock, "READY", 256, 0);
        }

        char commit_status[256];
        bzero(commit_status, 256);
        int receive2 = read(client_sock, commit_status, sizeof(commit_status));

        cout << endl
             << "Coordinator Message:" << commit_status<< endl;

        char message2[256];
        bzero(message2, 256);

        char transaction_after_updation[256];
        bzero(transaction_after_updation, 256);

        //Read transaction
        //If message is not equal to global commit
        if ((strcmp(commit_status, "ABORT")) == 0)
        {
            bzero(message1, 256);
            bzero(commit_status, 256);
            bzero(transaction_after_updation, 256);
            bzero(message2, 256);
        }
        else if ((strcmp(commit_status, "ABORT")) != 0)
        {
            int receive3 = read(client_sock, message2, sizeof(message2));
            cout << endl
                 << "Transaction details: " << message2 << endl;
        }
         if (((strcmp(commit_status, "COMMIT")) == 0) && ((strcmp(commit_status, "ABORT")) != 0))
        {
            char *split_transaction;
            float balance;
            split_transaction = strtok(message2, " ");
            if (split_transaction != NULL)
            {

                //performs required operations when a transaction contains "CREATE"
                if (strcmp(split_transaction, "CREATE") == 0)
                {
                    pthread_mutex_lock(&mutex);
                    char *one;
                    balance = atof(strtok(NULL, " "));
                    records[number] = balance;
                    int account_created = number;
                    number = number + 1;
                    sprintf(transaction_after_updation, "OK %d", account_created);
                    int s1 = send(client_sock, transaction_after_updation, sizeof(transaction_after_updation), 0);
                    pthread_mutex_unlock(&mutex);
                    cout << endl
                         << "Transaction Successfull!" << endl;
                }
                //performs required operations when a transaction contains "QUERY"
                else if (strcmp(split_transaction, "QUERY") == 0)
                {

                    pthread_mutex_lock(&mutex);

                    int account_tobechecked = atoi(strtok(NULL, " ")); //101
                    if (records.find(account_tobechecked) != records.end())
                    {
                        sprintf(transaction_after_updation, "OK %.2f", records[account_tobechecked]);

                        int s1 = send(client_sock, transaction_after_updation, sizeof(transaction_after_updation), 0);
                        cout << endl
                          << "Transaction Successfull!" << endl;
                    }
                    else
                    {
                        sprintf(transaction_after_updation, "ERR: Account %d does not exist", account_tobechecked);

                        int s1 = send(client_sock, transaction_after_updation, sizeof(transaction_after_updation), 0);
                        cout << endl
                             << "Transaction Unsuccessful!" << endl;
                    }

                    pthread_mutex_unlock(&mutex);
                }

                //performs required operations when a transaction contains "UPDATE"
                else if (strcmp(split_transaction, "UPDATE") == 0)
                {
                    pthread_mutex_lock(&mutex);
                    int accountNumber = atoi(strtok(NULL, " "));
                    float amountReceived = atof(strtok(NULL, " "));

                    if (records.find(accountNumber) != records.end())
                    {
                        records[accountNumber] = amountReceived;
                        sprintf(transaction_after_updation, "OK %.2f", records[accountNumber]);
                        int s = send(client_sock, transaction_after_updation, sizeof(transaction_after_updation), 0);
                        cout << endl
                             << "Transaction Successfull!" << endl;
                    }
                    else
                    {
                        sprintf(transaction_after_updation, "Err Account %d does not exist", accountNumber);
                        int s = send(client_sock, transaction_after_updation, sizeof(transaction_after_updation), 0);
                        cout << endl
                             << "Account Number not Found!" << endl;
                    }

                    pthread_mutex_unlock(&mutex);
                }

                else
                {
                    sprintf(transaction_after_updation, "Error: This transaction is not valid");
                    int s1 = send(client_sock, transaction_after_updation, sizeof(transaction_after_updation), 0);
                    cout << "Invalid Transaction!" << endl;
                }
            }
        }

        bzero(transaction_after_updation, 256);
    }

    close(client_sock);
}
int main(int argc, char *argv[])
{

    int sock_desc, serversocket, serverlen;
    int *sock1;
    struct sockaddr_in backend_server, cli;
    //create socket for backend server
    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    //create sockaddr_in structure
    backend_server.sin_family = AF_INET;
    backend_server.sin_addr.s_addr = INADDR_ANY;
    backend_server.sin_port = htons(atoi(argv[1]));

    //bind the socket
    setsockopt(sock_desc, SOL_SOCKET, SO_REUSEADDR, (char *)&backend_server, sizeof(backend_server));
    if (bind(sock_desc, (struct sockaddr *)&backend_server, sizeof(backend_server)) < 0)
    {
        cout << "Error while binding backend server" << endl;
        return 1;
    }

    //listen to accept connections from front end server
    listen(sock_desc, 50);
    cout << "Waiting for incoming connections: " << endl;

    //Accept Connections

    serverlen = sizeof(struct sockaddr_in);
    while (serversocket = accept(sock_desc, (struct sockaddr *)&cli, (socklen_t *)&serverlen))
    {
        sock1 = (int *)malloc(1);
        *sock1 = serversocket;
        pthread_t backendThread;
        pthread_create(&backendThread, NULL, Backend_server, (void *)sock1);
    }
    return 0;
}

/*References:
https://linux.die.net/man/2/setsockopt
https://github.com/intekmaster/Fault-Tolerant-Banking-Service
https://github.com/RashmiPrava21/OperatingSystems-Concurrent-Banking-Transactions
*/