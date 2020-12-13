#include <string>
#include <vector>
#include <cassert>
#include "include/db.hpp"
#include "random.h"

int main(int argc, char **argv)
{
    DB *db;
    unsigned int seed = 123;
    Random rng(seed);
    DB::CreateOrOpen(argv[1], &db);
    for (int i = 0; i < 10000; i++)
    {
        auto kv = rng.next();
        std::string v;
        db->Get(kv.first, &v);
        assert(kv.second.to_string() == v);
    }
    for (int i = 0; i < 26; i++)
    {
        char key_s[16];
        memset(key_s, 'a' + i, 16);
        std::string v;
        Slice k(key_s, 16);
        if (db->Get(k, &v) == Ok)
        {
            for (int i = 0; i < v.size() - 1; i++)
            {
                if (v[i] != v[i + 1])
                {
                    exit(1);
                }
            }
        }
    }
    return 0;
}