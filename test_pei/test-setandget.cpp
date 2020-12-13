#include <string>
#include <vector>
#include <cassert>
#include "db.hpp"

#include <unordered_map>
#include <iostream>
// #define _GLIBCXX_USE_CXX11_ABI 0

void get_rand_str(char *s, int num, uint32_t seed)
{
    //定义随机生成字符串表
    char str[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,./;\"'<>?";
    int i, lstr;
    char ss[2] = {0};
    lstr = strlen(str);                        //计算字符串长度
    srand(seed); //使用系统时间来初始化随机数发生器
    for (i = 0; i < num; i++)
    {                                            //按指定大小返回相应的字符串
        // sprintf(ss, "%c", str[(rand() % lstr)]); //rand()%lstr 可随机返回0-71之间的整数, str[0-71]可随机得到其中的字符
        s[i] = str[(rand() % lstr)];
        // strcat(s, ss);                           //将随机生成的字符串连接到指定数组后面
    }
}

int main(int argc, char **argv)
{
    DB *db;

    FILE* log_file = fopen("./test.log","wb");
    DB::CreateOrOpen(argv[1], &db, log_file);
    printf("test set CreateOrOpen\n");

    const uint16_t value_size = 1024;

    printf("init unordered_map\n");
    std::unordered_map<std::string, std::string> db_map;

    // const unsigned int key_num = 16;
    // char key_buf[key_num];
    // get_rand_str(key_buf, key_num);
    // std::string key_demo(key_buf, key_num);
    // printf("k:%s size=%uaaa\n", key_demo.c_str(), key_demo.length());

    // printf("init unordered_map successfully\n");
    // char key[4] = "aaa";
    // char value[5] = "bbbb";
    // db_map.insert(std::make_pair<std::string, std::string>(std::string(key, 3), std::string(value, 4)));

    // // printf("k:%s v:%s\n", std::string(key, 3).c_str(), db_map[std::string(key, 3)].c_str());

    // for(const auto & kv : db_map){
    //     printf("k:%s v:%saaa\n", kv.first.c_str(), kv.second.c_str());
    // }

    const uint32_t test_count = 1000000;
    const unsigned int key_num = 16;
    char key_buf[key_num];
    char value_buf[1024];
    for (uint32_t i = 0; i < test_count; i++)
    {
        srand((unsigned)time(NULL));
        get_rand_str(key_buf, key_num, i + rand());

        Slice k(key_buf, key_num);
        std::string gen_k = std::string(key_buf, key_num);
        
        unsigned int value_num = (rand() % 945) + 80;
        get_rand_str(value_buf, value_num, i + rand());
        Slice v(value_buf, value_num);
        std::string gen_v = std::string(value_buf, value_num);
        
        // printf("k:%s v:%saaa\n", gen_k.c_str(), gen_v.c_str());

        db->Set(k, v);

        db_map.insert(std::make_pair(gen_k, gen_v));

        // char key_s[16];
        // memset(key_s, 'a' + i, 16);
        // Slice k(key_s, 16);

        // printf("test-set key %s, %lld\n", k.to_string().c_str(), k.size());

        // char value_s[value_size];
        // memset(value_s, 'z' - i, value_size);
        // Slice v(value_s, value_size);

        // db->Set(k, v);
    }
    std::string va;
    uint32_t test_cnt = 0;
    for(const auto & kv : db_map){
        test_cnt++;
        db->Get(Slice((char *)kv.first.c_str(), kv.first.length()), &va);
        // printf("k:%s v:%s\n", kv.first.c_str(), kv.second.c_str());
        if(va != kv.second){
            printf("TEST%llu, Doesn't Matched!!!key=%s, v=%s,size=%u, gt=%s\n", test_cnt, kv.first.c_str(), va.c_str(), va.size(), kv.second.c_str());
            exit(0);
        }
    }
    printf("DBHASHSIZE=%llu, TEST%llu, Matched ALL Tests\n", db_map.size(), test_cnt);
    // for (int i = 0; i < test_count; i++)
    // {
    //     char key_s[16];
    //     memset(key_s, 'a' + i, 16);
    //     std::string v;
    //     Slice k(key_s, 16);
    //     if (db->Get(k, &v) == Ok){
    //         char value_s[value_size];
    //         memset(value_s, 'z' - i, value_size);
    //         int ret = memcmp(v.data(), value_s, value_size);
    //         if(ret == 0){
    //             printf("Matched!! test-get value %c, %u\n", 'z' - i, v.length());
    //         }else{
    //             printf("Doesn't Matched!!!\n", v.c_str(), v.length());
    //         }
    //     }else{
    //         printf("test-get key %s NotFound\n", k.to_string().c_str());
    //     }
    // }


    
    return 0;
}
