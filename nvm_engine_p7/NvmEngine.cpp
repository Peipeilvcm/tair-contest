#include "NvmEngine.hpp"
#include <sys/stat.h>


#define USE_LIBPMEM



Status DB::CreateOrOpen(const std::string &name, DB **dbptr, FILE *log_file) {
    return NvmEngine::CreateOrOpen(name, dbptr);
}

DB::~DB() {}


Status NvmEngine::CreateOrOpen(const std::string &name, DB **dbptr) {
    struct stat st = {0};
    if(stat(name.data(), &st) == -1) {
        mkdir(name.data(), 0777);
    }

    fprintf(stderr, "Opening database!\n");

    NvmEngine *nvm_engine_ptr = new NvmEngine(name);

    *dbptr = nvm_engine_ptr;

    return Ok;
}



NvmEngine::NvmEngine(const std::string &name){
    this->pEngine = new PEngine(name);
}

Status NvmEngine::Get(const Slice &key, std::string *value) {

  return pEngine->Get(key, value);
    
}

Status NvmEngine::Set(const Slice &key, const Slice &value) {

  pEngine->Set(key, value);

  return Ok;

}

NvmEngine::~NvmEngine() {
    delete pEngine;
    fprintf(stderr, "closing database\n");
}


