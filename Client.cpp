/*Client which will connect to front-end server */
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
int main(int argc, char *argv[])
{

    string tran;
    char input_tran[256];
    char output_tran[256];

    struct hostent *servername;
    int portno;
    portno = atoi(argv[1]);
    struct sockaddr_in frontend_server_addr;

    frontend_server_addr.sin_family = AF_INET;
    frontend_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    frontend_server_addr.sin_port = htons(portno);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    connect(sockfd, (struct sockaddr *)&frontend_server_addr, sizeof(frontend_server_addr));
    while (1)
    {
        //accepts the transactions details from user
        cout << endl;
        cout << "Enter the Transaction:" << endl;
        getline(cin, tran);
        strcpy(input_tran, tran.c_str());
        //Send transaction to front end server
         for (int i = 0; i < sizeof(input_tran); i++)
        {
            input_tran[i] = toupper(input_tran[i]);
        }
        int send_tran = send(sockfd, input_tran, sizeof(input_tran), 0);
        //Recieve transaction status
        int read_tran = read(sockfd, output_tran, sizeof(output_tran));

        if (strcmp(output_tran, "QUIT") == 0)
        {
            cout << "OK" << endl;
            cout << "Connection Terminated by foriegn host." << endl;
            break;
        }
        if (strcmp(output_tran, "ABORT") == 0)
        {
            cout << "Transaction Aborted!" << endl;
        }
        else
        {
            cout << output_tran << endl;
            bzero(output_tran, 256);
            bzero(input_tran, 256);
        }
    }

    close(sockfd);
    return 0;
}
