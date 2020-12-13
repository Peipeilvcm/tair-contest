#ifndef RANDOM_POOL
#define RANDOM_POOL
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

// #include <random>
#include <time.h>

#include "db.hpp"

class Random2
{
public:
    Random2() {}
    ~Random2() {}
    std::pair<Slice, Slice> next()
    {
        char key_buf[16];
        const unsigned int key_num = 16;
        get_rand_str(key_buf, key_num);

        srand((unsigned int)time((time_t *)NULL));
        char value_buf[1024];
        unsigned int value_num = (rand() % 945) + 80;
        get_rand_str(value_buf, value_num);
        
        return std::make_pair(Slice{key_buf, key_num}, Slice{value_buf, value_num});
    }

    void get_rand_str(char s[], int num)
    {
        //定义随机生成字符串表
        char *str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,./;\"'<>?";
        int i, lstr;
        char ss[2] = {0};
        lstr = strlen(str);                        //计算字符串长度
        srand((unsigned int)time((time_t *)NULL)); //使用系统时间来初始化随机数发生器
        for (i = 1; i <= num; i++)
        {                                            //按指定大小返回相应的字符串
            sprintf(ss, "%c", str[(rand() % lstr)]); //rand()%lstr 可随机返回0-71之间的整数, str[0-71]可随机得到其中的字符
            strcat(s, ss);                           //将随机生成的字符串连接到指定数组后面
        }
    }
};
#endif