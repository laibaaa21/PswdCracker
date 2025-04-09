#ifndef HELPERS_H
#define HELPERS_H
#include <vector>
#include <string>
#include <queue>
#include <functional>

void generate_keyspace_chunks(const std::string &charset, int max_length, int chunk_size, std::queue<std::vector<std::string>> &task_queue)
{
    std::vector<std::string> keyspace;
    std::function<void(std::string, int)> generate = [&](std::string prefix, int depth)
    {
        if (depth > max_length)
            return;
        if (!prefix.empty())
            keyspace.push_back(prefix);
        for (char c : charset)
        {
            generate(prefix + c, depth + 1);
        }
    };

    generate("", 0);

    for (size_t i = 0; i < keyspace.size(); i += chunk_size)
    {
        std::vector<std::string> chunk;
        for (int j = 0; j < chunk_size && i + j < keyspace.size(); ++j)
        {
            chunk.push_back(keyspace[i + j]);
        }
        task_queue.push(chunk);
    }
}
#endif