#include <string>
#include <vector>
#include <cassert>
#include "db.hpp"

int main(int argc, char **argv)
{
    DB *db;

    printf("test set CreateOrOpen\n");
    FILE * log_file =  fopen("./performance.log", "w");
	DB::CreateOrOpen(argv[1], &db, log_file);
    printf("insert 26\n");
    for (int i = 0; i < 26; i++)
    {
        char key_s[16];
        memset(key_s, 'a' + i, 16);
        Slice k(key_s, 16);

        printf("test-set key %s, %lld\n", k.to_string().c_str(), k.size());

        char value_s[1024];
        memset(value_s, 'z' - i, 80 + i * 36);
        Slice v(value_s, 80 + i * 36);

        db->Set(k, v);
    }
    
    return 0;
}