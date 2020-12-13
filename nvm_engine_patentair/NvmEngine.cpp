#include "NvmEngine.hpp"
#include <sys/stat.h>


#define USE_LIBPMEM

#include "coding.h"
#include "hash.h"
#include <iostream>
#include <thread>



Status DB::CreateOrOpen(const std::string &name, DB **dbptr, FILE *log_file) {
    return NvmEngine::CreateOrOpen(name, dbptr);
}

DB::~DB() {}


Status NvmEngine::CreateOrOpen(const std::string &name, DB **dbptr) {

    fprintf(stderr, "opening database\n");

    *dbptr = nullptr;

    NvmEngine *nvm_engine_ptr = new NvmEngine(name);

    uint32_t num_processor = sysconf(_SC_NPROCESSORS_CONF);

    nvm_engine_ptr->max_file_ = num_processor * 2;
    nvm_engine_ptr->table_size_ = num_processor;
    // Create skiplist tables
    for (uint32_t i = 0; i < nvm_engine_ptr->table_size_; ++i) {
        Table* table = new Table(Compare(nvm_engine_ptr->options_.comparator_));
        nvm_engine_ptr->tables_.push_back(table);
    }

    fprintf(stderr, "Create skiplist tables\n");

    // // Create data directory
    // std::string current_dir;
    // if (!Env::GetCurrentDir(&current_dir)) {
    //     return IOError;
    // }
    // std::string data_dir = current_dir + "/" + name;
    // if (!Env::FileExists(data_dir)) {
    //     auto s = Env::CreateDir(data_dir);
    //     if (!s) {
    //         return IOError;
    //     }
    // }

    // fprintf(stderr, "Create data directory\n");

    // Open data
    for (uint32_t i = 1; i <= nvm_engine_ptr->max_file_; i++) {
        std::string data_file = name + DataFileName + std::to_string(i);
        AepFile* file;
        // printf("%s\n", data_file.data());
        auto s = Env::NewFile(data_file, &file, nvm_engine_ptr->max_file_);
        if (s) {
            nvm_engine_ptr->data_files_.push_back(file);
        } else {
            return IOError;
        }
    }

    fprintf(stderr, "Opening data successfully\n");

    // Open index
    for (uint32_t i = 1; i <= nvm_engine_ptr->max_file_; i++) {
        std::string index_file = name + IndexFileName + std::to_string(i);
        AepFile* file;
        auto s = Env::NewFile(index_file, &file, nvm_engine_ptr->max_file_, true);
        if (s) {
        nvm_engine_ptr->index_files_.push_back(file);
        } else {
            return IOError;
        }
    }

    fprintf(stderr, "Open index successfully\n");

    nvm_engine_ptr->Recover();

    fprintf(stderr, "Recover successfully\n");
    

    *dbptr = nvm_engine_ptr;

    return Ok;
}



NvmEngine::NvmEngine(const std::string &name) : 
    dbname_(name), cursor_(0), max_file_(0){

    Options opts;
    opts.comparator_ = new BytewiseComparator();
    this->options_ = opts; //default setting

    this->set_counting = 0;
    this->get_counting = 0;
}

NvmEngine::~NvmEngine() {
    fprintf(stderr, "closing database\n");

    for (auto file : data_files_) {
        delete file;
    }
    for (auto file : index_files_) {
        delete file;
    }
    for (auto table: tables_) {
        delete table;
    }
}

Status NvmEngine::Get(const Slice &key, std::string *value) {

    // debug
    uint64_t counting = this->get_counting++;
    if(counting % 1000 == 0){
        // printf("get counting %d\n", counting);
    }


    uint32_t slot = (Hash(key.data()) & (table_size_ - 1));

    Index search_index(key.to_string()), internal_index;

    auto s = tables_[slot]->Get(search_index, &internal_index);

    if (likely(s)) {
        printf("A\n");
        if (unlikely(internal_index.KeySize() == 0 &&
            internal_index.ValueSize() == 0)) {
        // The key has been deleted.
        printf("The key has been deleted %s\n", key.to_string().c_str());
        *value = "";
        return NotFound;
        }
        char buf[internal_index.DataSize()];
        auto s = data_files_[internal_index.FileIndex()]->Read(internal_index.FileOffset(),
                                                            internal_index.DataSize(),
                                                            buf);
        if (likely(s)) {
            DecodeData(buf, internal_index.key_size_, internal_index.value_size_, value);
            printf("B\n");
            return Ok;
        } else {
            printf("C\n");
            return s ? Ok : NotFound;
        }
    } else {
        printf("D\n");
        return s ? Ok : NotFound;
    }
}

Status NvmEngine::Set(const Slice &key, const Slice &value) {

    uint32_t cursor = cursor_++;
    uint8_t file_index = (cursor & (max_file_ - 1));
    AepFile* data_file = data_files_[file_index];
    uint64_t file_offset;
    auto s = data_file->AppendData(key.to_string(), value.to_string(), &file_offset);
    if (likely(s)) {
        AepFile* index_file = index_files_[file_index];
        s = index_file->AppendIndex(key.to_string(), file_index, file_offset,
                    key.size(), value.size());
        if (likely(s)) {
            Index index(key.to_string(), file_index, file_offset, key.size(), value.size());
            int32_t slot = (Hash(key.data()) & (table_size_ - 1));
            tables_[slot]->Insert(index);
        }
    }

    // debug
    uint64_t counting = this->set_counting++;
    if(counting % 1000 == 0){
        // printf("set counting %d\n", counting);
    }
    return s ? Ok : NotFound;
}


bool NvmEngine::Recover() {

    printf("Starting Recover, max_file_=%d\n", max_file_);


  // Recover
    std::vector<std::thread > threads;
    for (uint32_t i = 0; i < max_file_; i++) {
        std::thread recover(&NvmEngine::IndexCallback, this, index_files_[i]);
        threads.push_back(std::move(recover));
    }

    for (uint32_t i = 0; i < max_file_; i++) {
        threads[i].join();
    }

    // for (uint32_t i = 0; i < max_file_; i++) {
    //     IndexCallback(index_files_[i]);
    // }

    return true;
}

void NvmEngine::IndexCallback(AepFile* file) {

    // printf("indexcallback A\n");
    uint64_t file_offset = file->FileOffset();

    if (file_offset != 0) { // Load index files into skiplist

        // printf("indexcallback B\n");
        uint64_t pos = 0;
        while(pos < file_offset) {
            char size_buf[sizeof(uint32_t)];
            auto s = file->Read(pos, sizeof(uint32_t), size_buf);
            // printf("indexcallback C s=%d\n", s);
            if (s) {
                pos += sizeof(uint32_t);

                // printf("indexcallback D\n");
                uint32_t index_size = DecodeFixed32(size_buf);

                // printf("indexcallback E index_size=%d\n", index_size);
                char index_buf[index_size];
                // printf("indexcallback F s=%d\n", s);
                auto s = file->Read(pos, index_size, index_buf);

                if (s) {
                    
                    pos += index_size;
                    Index index;
                    DecodeIndex(index_buf,
                            &index.key_,
                            &index.file_index_,
                            &index.file_offset_,
                            &index.key_size_,
                            &index.value_size_);
                    
                    int32_t slot = (Hash(index.key_.data()) & (table_size_ - 1));
                    tables_[slot]->Insert(index);
                }
            }
        }
    }
}


