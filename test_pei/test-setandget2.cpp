#include <string>
#include <vector>
#include <cassert>
#include "db.hpp"
#include "random2.h"
#include <unordered_map>

using namespace std;
int main(int argc, char **argv)
{
    DB *db;
    Random2 rng{};
    DB::CreateOrOpen(argv[1], &db);
    printf("test set CreateOrOpen\n");

    const uint32_t test_count = 1024*1024;

    unordered_map<string, string> hashmap;

    // set
    for (int i = 0; i < test_count; i++)
    {
        auto kv_ = rng.next();
        db->Set(kv_.first, kv_.second);
        hashmap.insert(make_pair(kv_.first.to_string(), kv_.second.to_string()));
    }

    // get
    // std::unordered_map<Slice, Slice>::iterator it;
    // bool flag = True;
    std::string v;
    for(unordered_map<string, string>::iterator kv=hashmap.begin(); kv!=hashmap.end();kv++){
        db->Get(Slice((char *)kv->first.c_str(), kv->first.size()), &v);
        if(v != kv->second){
            printf("Doesn't Matched!!!key=%s, v=%s,size=%u, gt=%s\n", kv->first.c_str(), v.c_str(), v.size(), kv->second.c_str());
            exit(0);
        }
    }
    
    return 0;
}
