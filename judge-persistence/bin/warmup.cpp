#include <string>
#include <vector>

#include "include/db.hpp"
#include "random.h"

int main(int argc, char **argv) {
    DB *db;
    unsigned int seed = 123;
    Random rng(seed);
    DB::CreateOrOpen(argv[1], &db);
    for (int i = 0; i < 10000; i++) {
        auto kv = rng.next();
        db->Set(kv.first, kv.second);
    }
    return 0;
}