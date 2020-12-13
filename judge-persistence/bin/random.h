#ifndef RANDOM_POOL
#define RANDOM_POOL
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <random>

#include "include/db.hpp"

class Random
{
public:
    Random(uint32_t seed, uint32_t max = 409600) {}
    Slice _make_slice(std::string s) {
        char* buf = new char[s.size()];
        memcpy(buf, s.c_str(), s.size());
        return Slice(buf, s.size());
    }
    std::string _make_random_string(const int len) {
        const char* alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        const auto alphabet_len = strlen(alphabet);

        std::string result;

        for (int i = 0; i < len; ++i)
            result += alphabet[rand() % alphabet_len];

        return result;	
    }
    std::pair<Slice, Slice> next()
    {
        srand(time(NULL));
        Slice k = _make_slice(_make_random_string(16));
        Slice v = _make_slice(_make_random_string(random()%(944) + 80));
        return std::make_pair(k, v);
    }
};
#endif