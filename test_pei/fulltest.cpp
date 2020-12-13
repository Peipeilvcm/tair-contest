#include <string>
#include <vector>
#include <cassert>
#include "db.hpp"
#include "random2.h"

int main(int argc, char **argv)
{
    DB *db;
    unsigned int seed = 123;
    Random rng(seed);
    DB::CreateOrOpen(argv[1], &db);
    printf("fulltest CreateOrOpen\n");
    for (int i = 0; i < 26; i++)
    {
        // auto kv = rng.next();
        // std::string v;

        // printf("fulltest %d, %s\n", i, kv.first.data());

        // db->Get(kv.first, &v);

        // assert(kv.second.to_string() == v);

        char key_s[16];
        memset(key_s, 'a' + i, 16);
        Slice k(key_s, 16);

        // printf("onceset key %s, %lld\n", k.to_string().c_str(), k.size());

        char value_s[80];
        memset(value_s, 'z' - i, 80);
        Slice v(value_s, 80);

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
                    printf("ERROR\n");
                    exit(1);
                }
            }
        }
    }
    return 0;
}