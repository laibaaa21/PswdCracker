#include <iostream>
#include <vector>
#include <sstream>
#include <netinet/in.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include "config.h"

std::string sha256(const std::string &str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)str.c_str(), str.size(), hash);
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
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr = {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_HOST.c_str(), &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

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

    close(sock);
    return 0;
}
