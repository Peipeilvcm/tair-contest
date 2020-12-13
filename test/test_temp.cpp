#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<iostream>
#include<vector>

#include"xxhash.h"

struct A{
    char k[128];
    uint32_t b;
};

struct AA{
    uint16_t b;
    char k[128];
    
};

long gcd(long m, long n)
{
    for(;;) {
        if(n == 0)
            return m;
        long temp = m % n;
        m = n;
        n = temp;
    }
}
 
long lcm(long a, long b)
{
//    return a * b / gcd(a, b);
   return a / gcd(a, b) * b;
 
}

int main(){

    printf("hash = %llu\n", XXH32("aaaaaaaaaaaaaaaa", 16, 0));

    uint8_t a = 0x7F;
    uint8_t b = 0xFF;

    A x[3];

    A* w;

    bool bbb[1024];
    std::vector<bool> vvv(false, 1024);

    uint32_t index_offsets[3];
    memset(index_offsets, 0xFF, sizeof(index_offsets));
    uint32_t k = 0;
    while (~index_offsets[k]) // 非空
    {
        ++k;
        puts("非空");
    }

    // uint16_t size = 127;
    uint8_t chunk_size = 4;
    int r;
    for(uint16_t i = 102; i <= 1046; i += 1){
        r = ((i + (1<<4) - 1) >> 4) << 4;
        std::cout << r << std::endl;
    }
    

    // uint16_t a = 0x7FFF;
    // uint16_t b = 0xFFFF;

    std::cout<<lcm(132,256);
    printf("a=%d,b=%d, %s, %ld,%ld, %u ,%u, %ld", a, b, a>b ? "a>b" : "a<b", sizeof(uint32_t),sizeof(AA), sizeof(w), sizeof(bbb), sizeof(x));

    return 0;
}