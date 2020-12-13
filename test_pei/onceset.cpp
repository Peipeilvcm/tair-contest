#include <libpmem.h>

#include <cassert>
#include <cstring>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "db.hpp"

extern "C"
{
    __attribute__((__used__)) int trigger_exit()
    {
        exit(0);
    }
}

int main(int argc, char **argv)
{
    DB *db;
    const char *const db_name = argv[1];

    DB::CreateOrOpen(db_name, &db);
    char key_s[16];
    memset(key_s, 'a' + atoi(argv[2]), 16);
    Slice k(key_s, 16);

    printf("onceset key %s, %s, %lld\n", atoi(argv[2]), k.data(), k.size());

    char value_s[80];
    memset(value_s, 'x', 80);
    Slice v(value_s, 80);

    printf("onceset value %s, %lld\n",  v.data(), v.size());

    db->Set(k, v);
    trigger_exit();
    return 0;
}