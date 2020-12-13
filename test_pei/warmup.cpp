#include <string>
#include <vector>

#include "db.hpp"
#include "random2.h"

int main(int argc, char **argv) {
    DB *db;
    unsigned int seed = 123;
    Random rng(seed);
    DB::CreateOrOpen(argv[1], &db);
    printf("warmup CreateOrOpen Successfully\n");
    for (int i = 0; i < 10000; i++) {
        auto kv = rng.next();
        db->Set(kv.first, kv.second);
        printf("warmup %d, %s, %s\n", i, kv.first.data(), kv.second.data());
    }
    return 0;
}