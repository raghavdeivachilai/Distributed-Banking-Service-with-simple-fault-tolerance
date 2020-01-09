# Distributed Banking Service with simple fault tolerance:

I have implemented a simple, distributed banking system that
permits multiple concurrent operations and exhibits fault tolerance. In order to implement this,
I have created 3 programs as follows:

### Client: 
This program gets the transaction details from the user and sends it to the frontend
server. Once the transaction gets processed by the frontend and backend servers, the results
are sent to the client which is displayed to the client.
Frontend server: This program accepts the connections from the client and passes the
transaction details to the 3 backend servers. The frontend client asks whether the all the
backend servers commit or abort and takes an action accordingly. In case of Commit, backend
servers process the requests and send the output to this program. As a result, frontend sends
the responses to client. In case of Abort, the backend servers send ABORT which is passed to the
client.

### Backend server:
As mentioned in the project description, I have created 3 backend servers
which gets all the transaction details from the frontend server and performs the required
actions. The backend servers maintain atomicity and perform an operation only if all the
backend servers accept otherwise does not perform it and sends Abort.

We will execute the project by first running all the backend servers with the port number as specified in
the readme file. Then, frontend server is started which again waits for the incoming connections from
the client. Finally, client is started, which asks for the transaction details. Once user provides the first
transaction detail as “CREATE 100.00”, client sends these details to frontend server.
Frontend server then accepts the connections and sends this connection details to backend servers. A
separate thread is created for each client connections. All the backend servers send a “VOTE” status and
simultaneously frontend server counts the number of active backend servers. Once the frontend server
gets all the backend servers' status, it asks all of them to commit the transaction (to show the case
when the backend server aborts, I have written a random number generator which aborts the
transaction whenever 0 comes). As mentioned in the project description, all the transactions should be
in uppercase, so frontend converts them and sends it to the backend servers. Also, it checks if the
transaction is “QUIT”. If it is, then it sends an “OK” message to the client and the client socket is closed.
Backend servers handle all the transactions for “CREATE”, “UPDATE” and “QUERY”. There are separate
functions for each of these transactions. I have used a Map data structure used to store the account
number and balance details.

I have created 3 programs Client.cpp, server.cpp and frontendclient.cpp. All the files should be run in
separate terminals. Each file should be run as follows:

## Client.cpp:
The client.cpp file contains the front-end implementation. It should be run at the end.
Compile: g++ Client.cpp -o client -pthread
Run the executable: ./client <portnumber> (same port number as the port number you are
running frontendserver.cpp) ./client 7000
After running this executable, the program will get connected to the front-end server and ask the user
to provide the required inputs to carry out the required transactions (CREATE, QUERY, UPDATE or QUIT).
The client program shows the output that the user can see.
Frontendserver.cpp:
The frontendserver.cpp contains the coordinator implementation of the code. You can run it as follows:
Compile: g++ frontendserver.cpp -o front –pthread
Run the executable: ./front <port_number> ./front 7000
After running this executable, the program will pass the instructions from the client to the backend
servers and the results from the servers to the client.

## Server.cpp
The server.cpp file contains the backend server implementation. The server file must be run in 3
separate terminals to emulate 3 separate servers. Each file must be run as follows:
Compile: g++ server.cpp -o server –pthread
Run the executable: ./server 4000
./server 5000
./server 6000
The server.cpp file is now ready to accept the transactions from the client.
First open the three server.cpp files in three terminals as mentioned above and in another terminal
open the and in another terminal using ./frontend 7000 and in another terminal open ./client 7000 and
execute the commands CREATE, QUERY,UPDATE,DELETE.
