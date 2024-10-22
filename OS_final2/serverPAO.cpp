
#include <poll.h>
#include <algorithm>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include "graph.hpp"
#include "MST_Strategy.hpp"
#include "MST_Factory.hpp"
#include "serverAPI.hpp"
#include "PipelineActiveObject.hpp"
#include <memory>
#include <mutex>
#include <signal.h>  

#define PORT "9036"   // Server listening port
#define WELCOME_MSG_SIZE 480

using namespace std;

/**
 * A struct to hold the Graph object and the message string to be sent back to the client.
 * It will be used by the PAO object to process tasks for each client.
 */
struct ClientData {
    Graph* graph;
    string message;
    int clientSocket;
};

// Global variables:
map<int, pair<Graph*, ClientData*>> clientDataMap;  // Mapping client socket to graph and client data
map<int, mutex> clientMutexes;  // Mapping client socket to its mutex
struct pollfd* pollFds;  // Array of pollfd structs for managing file descriptors
int pollFdCount = 0;
PAO* pao = nullptr;
mutex globalMutex;

/**
 * Function to handle MST (Minimum Spanning Tree) request from the client.
 * It creates a new ClientData object, sets it in the client data map, and
 * schedules the task to be executed by the PAO.
 */
std::pair<std::string, Graph*> handleMSTRequest(Graph* graph, int clientSocket, const std::string& strategy) {
    Graph* mstGraph = nullptr;
    ClientData* clientData = nullptr;

    {
        unique_lock<mutex> lock(clientMutexes[clientSocket]);

        if (clientDataMap[clientSocket].second != nullptr) {  // Cleanup previous data
            if (clientDataMap[clientSocket].second->graph != nullptr) {
                delete clientDataMap[clientSocket].second->graph;
            }
            delete clientDataMap[clientSocket].second;
        }

        // Create and set new ClientData
        clientDataMap[clientSocket].second = new ClientData{graph, strategy, clientSocket};
        clientData = clientDataMap[clientSocket].second;

        // Create MST strategy and generate the MST
        MST_Strategy* mstStrategy = MST_Factory::getInstance()->createMST(clientData->message);
        mstGraph = (*mstStrategy)(clientData->graph);
    }

    clientData->graph = mstGraph;
    clientData->message = "MST created using " + clientData->message + " strategy\n";

    pao->addTask(clientData);  // Schedule the task in the PAO
    std::cout << "Client " << clientSocket << " requested to find MST of the graph" << std::endl;

    return {"", nullptr};
}

/**
 * Signal handler to clean up resources when the server is interrupted (e.g., by CTRL+C).
 */
void handleSignal(int signal) {
    {
        for (auto& clientMtx : clientMutexes) {
            clientMtx.second.lock();
        }

        cout << "\nServer: Cleaning up resources..." << endl;

        // Clean up graphs and client data
        for (auto& clientEntry : clientDataMap) {
            if (clientEntry.second.first != nullptr) {
                delete clientEntry.second.first;  // Free the graph
            }
            if (clientEntry.second.second != nullptr) {
                if (clientEntry.second.second->graph != nullptr) {
                    delete clientEntry.second.second->graph;  // Free the graph in the ClientData
                }
                delete clientEntry.second.second;  // Free the ClientData
            }
        }

        // Close and free all file descriptors
        for (int i = 0; i < pollFdCount; i++) {
            if (pollFds[i].fd != -1) {
                close(pollFds[i].fd);
            }
        }

        free(pollFds);

        if (pao != nullptr) {
            delete pao;  // Free PAO object
        }

        cout << "Server: Resources cleaned. Goodbye!" << endl;

        for (auto& clientMtx : clientMutexes) {
            clientMtx.second.unlock();
        }
    }
    exit(0);
}

// Main server function
int main(void) {
    // Define the functions to be executed by the PAO for each client
    std::vector<std::function<void(void*)>> taskFunctions = {
        [](void* clientData) {
            ClientData* data = (ClientData*)clientData;
            unique_lock<mutex> lock(clientMutexes[data->clientSocket]);
            data->message += "Total weight of edges: " + std::to_string(data->graph->totalWeight()) + "\n";
        },
        [](void* clientData) {
            ClientData* data = (ClientData*)clientData;
            unique_lock<mutex> lock(clientMutexes[data->clientSocket]);
            data->message += data->graph->longestPath() + "\n";
        },
        [](void* clientData) {
            ClientData* data = (ClientData*)clientData;
            unique_lock<mutex> lock(clientMutexes[data->clientSocket]);
            data->message += "Average distance between vertices: " + std::to_string(data->graph->avgDistance()) + "\n";
        },
        [](void* clientData) {
            ClientData* data = (ClientData*)clientData;
            unique_lock<mutex> lock(clientMutexes[data->clientSocket]);
            data->message += "Shortest paths:\n" + data->graph->allShortestPaths() + "\n";
        },
        [](void* clientData) {
            ClientData* data = (ClientData*)clientData;
            unique_lock<mutex> lock(clientMutexes[data->clientSocket]);
            if (send(data->clientSocket, data->message.c_str(), data->message.size(), 0) < 0) {
                perror("send");
            }
        }
    };

    // Initialize PAO with the task functions
    pao = new PAO(taskFunctions);
    pao->start();  // Start the PAO processing

    const vector<string> graphCommands = {"newgraph", "newedge", "removeedge", "mst"};
    const vector<string> mstAlgorithms = {"prim", "kruskal", "tarjan", "boruvka"};

    int newConnectionFd;
    struct sockaddr_storage clientAddr;
    socklen_t addrSize;
    char recvBuffer[256] = {0};
    char clientIP[INET6_ADDRSTRLEN] = {0};

    int pollArraySize = 5;
    int listenerSocket = getListenerSocket();
    if (listenerSocket == -1) {
        fprintf(stderr, "Error obtaining listener socket\n");
        exit(1);
    }

    // Initialize poll array and add the listener socket
    pollFds = (struct pollfd*)malloc(sizeof(*pollFds) * pollArraySize);
    memset(pollFds, 0, sizeof(*pollFds) * pollArraySize);
    pollFds[0].fd = listenerSocket;
    pollFds[0].events = POLLIN;
    pollFdCount = 1;

    signal(SIGINT, handleSignal);  // Set up signal handler for server interruption

    // Server main loop
    while (true) {
        int pollResult = poll(pollFds, (size_t)pollFdCount, -1);
        if (pollResult == -1) {
            perror("poll");
            exit(1);
        }

        for (int i = 0; i < pollFdCount; i++) {
            if (pollFds[i].revents & POLLIN) {
                if (pollFds[i].fd == listenerSocket) {
                    addrSize = sizeof(clientAddr);
                    newConnectionFd = accept(listenerSocket, (struct sockaddr*)&clientAddr, &addrSize);
                    if (newConnectionFd == -1) {
                        perror("accept");
                    } else {
                        add_to_pfds(&pollFds, newConnectionFd, &pollFdCount, &pollArraySize);
                        clientDataMap[newConnectionFd] = {nullptr, nullptr};  // Initialize client data
                        printf("New connection from %s on socket %d\n",
                               inet_ntop(clientAddr.ss_family, getInAddr((struct sockaddr*)&clientAddr),
                                         clientIP, INET6_ADDRSTRLEN), newConnectionFd);
                        if (send(newConnectionFd, welcomeMsg, sizeof(welcomeMsg), 0) < 0) {
                            perror("send");
                        }
                    }
                } else {
                    int bytesRead = recv(pollFds[i].fd, recvBuffer, sizeof recvBuffer, 0);
                    int clientFd = pollFds[i].fd;

                    if (bytesRead <= 0) {
                        if (bytesRead == 0) {
                            printf("Client on socket %d disconnected\n", clientFd);
                        } else {
                            perror("Error receiving data from client");
                        }
                        close(clientFd);
                        del_from_pfds(pollFds, i, &pollFdCount);
                        {
                            unique_lock<mutex> lock(clientMutexes[clientFd]);
                            if (clientDataMap[clientFd].first != nullptr) {
                                delete clientDataMap[clientFd].first;  // Free the graph
                            }
                            if (clientDataMap[clientFd].second != nullptr) {
                                if (clientDataMap[clientFd].second->graph != nullptr) {
                                    delete clientDataMap[clientFd].second->graph;
                                }
                                delete clientDataMap[clientFd].second;  // Free ClientData
                            }
                        }
                        clientDataMap.erase(clientFd);
                        clientMutexes.erase(clientFd);
                    } else {
                        string received(recvBuffer, bytesRead);
                        received.erase(received.find_last_not_of(" \n\r\t") + 1);
                        stringstream ss(received);
                        string command, subCommand;
                        ss >> command >> subCommand;

                        if (command == graphCommands[0]) {  // "newgraph"
                            int vertexCount = stoi(subCommand);
                            auto graph = new Graph(vertexCount);
                            {
                                unique_lock<mutex> lock(globalMutex);
                                if (clientDataMap[clientFd].first != nullptr) {
                                    delete clientDataMap[clientFd].first;  // Free the old graph
                                }
                                clientDataMap[clientFd].first = graph;  // Set the new graph
                                clientMutexes[clientFd].lock();
                            }
                            clientMutexes[clientFd].unlock();
                        }

                    }
                }
            }
        }
    }
    return 0;
}
