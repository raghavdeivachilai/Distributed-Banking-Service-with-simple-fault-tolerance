/*This is front end Server which will accept the connection from client and commit or abort the transaction*/
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

using namespace std;

//thread for each message from client

pthread_mutex_t mutex;
int portarr[] = {4000, 5000, 6000};
void *Coordinator(void *sockdesc)
{
    int client_socket = *(int *)sockdesc;
    int backendserver[3];
    int servercon_port[3];
    servercon_port[0] = htons(portarr[0]);
    servercon_port[1] = htons(portarr[1]);
    servercon_port[2] = htons(portarr[2]);
    int total_servers = 0;
    int servers_online[3] = {0};

    for (int i = 0; i < 3; i++)
    {
        struct sockaddr_in backend_server_addr;
        backend_server_addr.sin_family = AF_INET;
        backend_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        backend_server_addr.sin_port = ((servercon_port[i]));

        //array of sockets to connect to multiple servers one by one

        backendserver[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (backendserver[i] < 0)
        {
            cout << "Error from thread - Socket creation failed for backend server" << endl;
            return 0;
        }

        //Connect to servers one by one
        if (connect(backendserver[i], (struct sockaddr *)&backend_server_addr, sizeof(backend_server_addr)) >= 0)
        {
            servers_online[i] = 1;
            total_servers++;
        }
    }
    while (true)
    {

        char transaction_msg[256];
        char transaction_status[256];

        //Read from client
        int r1 = read(client_socket, transaction_msg, sizeof(transaction_msg));
        cout << endl;
        cout << "Transaction to be processed: " << transaction_msg << endl;

        if (strcmp(transaction_msg, "QUIT") == 0)
        {
            int s = send(client_socket, "QUIT", sizeof(4), 0);

            close(client_socket);
            break;
        }

        //ask the backend servers to provide their status
        for (int i = 0; i < 3; i++)
        {
            if (servers_online[i] == 1)
            {
                int status = send(backendserver[i], "VOTE", 256, 0);
            }
        }

        //Recieve status reply from the backend servers
        struct timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        servers_online[3] = {0};
        char buffer1[256];
        int count = 0;
        int abcount = 0;

        for (int i = 0; i < 3; i++)
        {
            if (setsockopt(backendserver[i], SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
            {
                cout << "Timeout error" << endl;
                return 0;
            }
            while (recv(backendserver[i], buffer1, 256, 0) > 0)
            {
                if (strcmp(buffer1, "ABORT") == 0)
                {

                    abcount++;
                }
                count++;
                servers_online[i] = 1;
            }
        }
        if (abcount > 0)
        {
            char arr[] = "ABORT";
            for (int i = 0; i < 3; i++)
            {
                send(backendserver[i], arr, sizeof(arr), 0);
            }
            int s = send(client_socket, arr, sizeof(arr), 0);
            cout << "Transaction Aborted" << endl;
        }
        else
        {
            cout << "Active backend servers: " << count;

            if (count == 0)
            {
                cout << "No active backend servers" << endl;
                sprintf(transaction_status, "ERROR: No Server is active.Please try after sometime");
                int s = send(client_socket, transaction_status, sizeof(transaction_status), 0);
            }
            else
            {
                if (count <= total_servers)
                {
                    char buffer2[256];
                    int j = 0;
                    while (recv(backendserver[j], buffer2, 256, 0) != 0)
                    {
                        if (j == 3)
                        {
                            break;
                        }
                        if (servers_online[j] == 1)
                        {
                            int s3 = send(backendserver[j], "COMMIT", 256, 0);

                            total_servers++;
                        }
                        j++;
                    }
                    char buffer3[256];
                    total_servers = 0;
                    for (int i = 0; i < 3; i++)
                    {

                        if (recv(backendserver[i], buffer3, 256, 0) != 0)
                        {
                            if (servers_online[i] == 1)
                            {
                                int s4 = send(backendserver[i], transaction_msg, sizeof(transaction_msg), 0);
                                total_servers++;
                            }
                        }
                        else
                        {
                            servers_online[i] = 0;
                        }
                    }

                    //Receive the updated transaction from the backend servers
                    total_servers = 0;

                    char buffer4[256];
                    bzero(transaction_status, 256);
                    for (int i = 0; i < 3; i++)
                    {

                        if (recv(backendserver[i], transaction_status, 256, 0) != 0)
                        {
                            if (servers_online[i] == 1)
                            {

                                total_servers++;
                            }
                        }
                        else
                        {
                            servers_online[i] = 0;
                        }
                    }
                    cout << endl;
                    int s3 = send(client_socket, transaction_status, sizeof(transaction_status), 0);
                }
            }

            bzero(transaction_msg, 256);
            bzero(buffer1, 256);
        }
    }

    close(client_socket);
}

int main(int argc, char *argv[])
{

    int sock_desc, cli_sock, cli_len;
    int *sock;
    struct sockaddr_in frontend, cli, server_main;

    //create front end TCP socket
    sock_desc = socket(AF_INET, SOCK_STREAM, 0);

    //Prepare sockaddr_in structure, get port from command line argument
    frontend.sin_family = AF_INET;
    frontend.sin_addr.s_addr = INADDR_ANY;
    frontend.sin_port = htons(atoi(argv[1]));

    //bind the front end  socket
    setsockopt(sock_desc, SOL_SOCKET, SO_REUSEADDR, (char *)&frontend, sizeof(frontend));

    bind(sock_desc, (struct sockaddr *)&frontend, sizeof(frontend));
    cout << "Coordinator is Ready!" << endl;
    //listen
    listen(sock_desc, 100);

    //Accept Connections
    cli_len = sizeof(struct sockaddr_in);

    //For each client, create a new thread and assigning a thread handle for each client
    while (cli_sock = accept(sock_desc, (struct sockaddr *)&cli, (socklen_t *)&cli_len))
    {
        cout << "Connection accepted" << endl;
        sock = (int *)malloc(1);
        *sock = cli_sock;
        pthread_t bankThread;
        pthread_create(&bankThread, NULL, Coordinator, (void *)sock);
    }

    return 0;
}
