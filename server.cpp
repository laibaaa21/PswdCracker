#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <unordered_map>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "config.h"
#include "helpers.h"

std::queue<std::vector<std::string>> task_queue;

void handle_worker(int client_socket, sockaddr_in addr)
{
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    int port = ntohs(addr.sin_port);
    std::string worker_id = std::string(ip) + ":" + std::to_string(port);

    std::cout << "[+] Worker connected: " << worker_id << std::endl;

    while (!task_queue.empty())
    {
        auto chunk = task_queue.front();
        task_queue.pop();
        std::string chunk_data;
        for (auto &s : chunk)
            chunk_data += s + " ";
        send(client_socket, chunk_data.c_str(), chunk_data.size(), 0);

        char buffer[1024];
        int valread = recv(client_socket, buffer, 1024, 0);
        if (valread <= 0)
            break;
        buffer[valread] = '\0';

        std::string data(buffer);
        if (data.find("FOUND") == 0)
        {
            std::string found_pw = data.substr(6);
            std::cout << "[✓] Password found by " << worker_id << ": " << found_pw << std::endl;
            exit(0); // Stop all
        }

        std::cout << "[ ] " << worker_id << " tried " << chunk.size() << " passwords" << std::endl;
    }

    close(client_socket);
}

int main()
{
    generate_keyspace_chunks(CHARSET, MAX_PASSWORD_LENGTH, CHUNK_SIZE, task_queue);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);
    std::cout << "[*] Server listening..." << std::endl;

    while (true)
    {
        sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        std::thread(handle_worker, client_socket, client_addr).detach();
    }

    close(server_fd);
    return 0;
}
