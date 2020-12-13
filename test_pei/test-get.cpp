#include <string>
#include <vector>
#include <cassert>
#include "db.hpp"

int main(int argc, char **argv)
{
    DB *db;
    FILE * log_file =  fopen("./performance.log", "w");
	DB::CreateOrOpen(argv[1], &db, log_file);
    for (int i = 0; i < 26; i++)
    {
        char key_s[16];
        memset(key_s, 'a' + i, 16);
        std::string v;
        Slice k(key_s, 16);
        if (db->Get(k, &v) == Ok){
            char value_s[1024];
            memset(value_s, 'z' - i, 80 + i * 36);
            int ret = memcmp(v.data(), value_s, 80 + i * 36);
            if(ret == 0){
                printf("Matched!! test-get value %c, %u\n", 'z' - i, v.length());
            }else{
                printf("Doesn't Matched!!!\n", v.c_str(), v.length());
            }

            db->Get(k, &v);
            ret = memcmp(v.data(), value_s, 80 + i * 36);
            if(ret == 0){
                printf("Matched!! test-get value %c, %u\n", 'z' - i, v.length());
            }else{
                printf("Doesn't Matched!!!\n", v.c_str(), v.length());
            }

            db->Get(k, &v);
            ret = memcmp(v.data(), value_s, 80 + i * 36);
            if(ret == 0){
                printf("Matched!! test-get value %c, %u\n", 'z' - i, v.length());
            }else{
                printf("Doesn't Matched!!!\n", v.c_str(), v.length());
            }

            db->Get(k, &v);
            ret = memcmp(v.data(), value_s, 80 + i * 36);
            if(ret == 0){
                printf("Matched!! test-get value %c, %u\n", 'z' - i, v.length());
            }else{
                printf("Doesn't Matched!!!\n", v.c_str(), v.length());
            }

            db->Get(k, &v);
            ret = memcmp(v.data(), value_s, 80 + i * 36);
            if(ret == 0){
                printf("Matched!! test-get value %c, %u\n", 'z' - i, v.length());
            }else{
                printf("Doesn't Matched!!!\n", v.c_str(), v.length());
            }

        }else{
            printf("test-get key %s NotFound\n", k.to_string().c_str());
        }
    }
    return 0;
}