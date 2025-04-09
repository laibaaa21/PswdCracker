#include <iostream>
#include <vector>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif
#include <openssl/sha.h>
#include "config.h"

std::string sha256(const std::string &str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(str.c_str()), str.size(), hash);
    char buf[65];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(buf + (i * 2), "%02x", hash[i]);
    buf[64] = 0;
    return std::string(buf);
}

std::string try_chunk(const std::vector<std::string> &chunk, const std::string &target_hash)
{
    for (auto &word : chunk)
    {
        if (sha256(word) == target_hash)
            return word;
    }
    return "";
}

int main()
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }
#endif

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        std::cerr << "Socket creation failed.\n";
        return 1;
    }

    sockaddr_in serv_addr = {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_HOST.c_str(), &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection to server failed.\n";
        return 1;
    }

    while (true)
    {
        char buffer[4096];
        int valread = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (valread <= 0)
            break;
        buffer[valread] = '\0';

        std::stringstream ss(buffer);
        std::string word;
        std::vector<std::string> chunk;
        while (ss >> word)
            chunk.push_back(word);

        std::string result = try_chunk(chunk, TARGET_HASH);
        if (!result.empty())
        {
            std::string msg = "FOUND " + result;
            send(sock, msg.c_str(), msg.size(), 0);
            break;
        }
        else
        {
            send(sock, "NOTFOUND", 8, 0);
        }
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return 0;
}
