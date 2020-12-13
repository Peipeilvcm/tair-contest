#include "PEngine.h"
#include "params.h"
#include <sys/time.h>      //添加头文件

milliseconds now() {
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch());
}

static thread_local std::unique_ptr<char> readBuffer(static_cast<char *> (memalign((size_t) getpagesize(), 4096)));

KeyValueLog *keyValueLogs[LOG_NUM];
KVFiles *kvFiles[FILE_NUM];
SortLog **sortLogs;
u_int64_t *sortKeysArray;
u_int16_t *sortValuesArray;

PMutex logMutex[LOG_NUM];

char *valueCache;
char *reserveCache;
std::atomic_flag readDiskFlag = ATOMIC_FLAG_INIT;

// PCond rangeCacheFinish[CACHE_NUM];
// std::atomic<int> rangeCacheCount[CACHE_NUM];

// PCond readDiskFinish[CACHE_NUM];

bool isCacheReadable[CACHE_NUM];
bool isCacheWritable[CACHE_NUM];
int currentCacheLogId[CACHE_NUM];

PMutex readDiskLogIdMtx;
// int readDiskLogId = 0;
// int rangeAllCount = 0;

int totalNum = 0;
bool unEnlarge = true;

PEngine::PEngine(const string &path){
    this->start = now();
    milliseconds t0 = this->start;
    set_count = 0;
    get_count = 0;

    // init
    std::ostringstream ss;
    ss << path << "/value-0";
    string filePath = ss.str();

    valueCache = nullptr;
    sortLogs = nullptr;

    int num_log_per_file = LOG_NUM / FILE_NUM;

    if (access(filePath.data(), 0) != -1) {
        sortLogs = static_cast<SortLog **>(malloc(LOG_NUM * sizeof(SortLog *)));
        sortKeysArray = (u_int64_t *) malloc(SORT_LOG_SIZE * LOG_NUM * sizeof(u_int64_t));
        sortValuesArray = (u_int16_t *) malloc(SORT_LOG_SIZE * LOG_NUM * sizeof(u_int16_t));

        valueCache = static_cast<char *> (memalign((size_t) getpagesize(), CACHE_SIZE * CACHE_NUM));
        reserveCache = valueCache + (ACTIVE_CACHE_NUM * CACHE_SIZE);

        for (int fileId = 0; fileId < FILE_NUM; fileId++) {
            kvFiles[fileId] = new KVFiles(path, fileId,
                                            true,
                                            VALUE_LOG_SIZE * num_log_per_file,
                                            KEY_LOG_SIZE * num_log_per_file,
                                            BLOCK_SIZE * num_log_per_file);
        }

        for (int logId = 0; logId < LOG_NUM; logId++) {
            int fileId = logId % FILE_NUM;
            int slotId = logId / FILE_NUM;
            keyValueLogs[logId] = new KeyValueLog(path, logId,
                                                    kvFiles[fileId]->getValueFd(),
                                                    slotId * VALUE_LOG_SIZE,
                                                    kvFiles[fileId]->getBlockBuffer() + slotId * BLOCK_SIZE,
                                                    kvFiles[fileId]->getKeyBuffer() + slotId * NUM_PER_SLOT);
            sortLogs[logId] = new SortLog(sortKeysArray + SORT_LOG_SIZE * logId,
                                            sortValuesArray + SORT_LOG_SIZE * logId);
        }

        for (int i = 0; i < CACHE_NUM; i++) {
            isCacheReadable[i] = false;
            isCacheWritable[i] = true;
            currentCacheLogId[i] = -1;
        }

        printf("Open files complete. time spent is %lims\n", (now() - t0).count());
        milliseconds t1 = now();

        //64线程读keylog
        std::thread t[RECOVER_THREAD];
        for (int i = 0; i < RECOVER_THREAD; i++) {
            t[i] = std::thread([i, this] {
                u_int64_t k;
                int flag = 0;
                for (int logId = i; logId < LOG_NUM; logId += RECOVER_THREAD) {
                    while (keyValueLogs[logId]->getKey(k))
                        sortLogs[logId]->put(k);
                    if (flag % 2 == 0)
                        sortLogs[logId]->quicksort();
                    flag++;
                    keyValueLogs[logId]->recover((size_t) sortLogs[logId]->size());
                }
            });
        }

        for (auto &i : t) {
            i.join();
        }


        printf("read keylogs complete. time spent is %lims\n", (now() - t1).count());
        milliseconds t2 = now();
        //4线程读cache
        std::thread t_cache[PREPARE_CACHE_NUM];

        for (int i = 0; i < PREPARE_CACHE_NUM; i++) {
            t_cache[i] = std::thread([i, this] {
                auto cache = reserveCache + i * CACHE_SIZE;
                keyValueLogs[i]->readValue(0, cache, (size_t) CACHE_SIZE);
            });
        }

        //64线程排序
        for (int i = 0; i < RECOVER_THREAD; i++) {
            t[i] = std::thread([i, this] {
                u_int64_t k;
                int flag = 0;
                for (int logId = i; logId < LOG_NUM; logId += RECOVER_THREAD) {
                    if (flag % 2 != 0)
                        sortLogs[logId]->quicksort();
                    flag++;
                }
            });
        }

        for (auto &i : t) {
            i.join();
        }

        for (auto &i : t_cache) {
            i.join();
        }

        printf("sort and read cache complete. time spent is %lims\n", (now() - t2).count());

        for (int logId = 0; logId < LOG_NUM; logId++)
            totalNum += sortLogs[logId]->size();

        if (totalNum < RANGE_THRESHOLD) {
            unEnlarge = false;
        } 
    } else {
        for (int fileId = 0; fileId < FILE_NUM; fileId++) {
            kvFiles[fileId] = new KVFiles(path, fileId,
                                            false,
                                            VALUE_LOG_SIZE * num_log_per_file,
                                            KEY_LOG_SIZE * num_log_per_file,
                                            BLOCK_SIZE * num_log_per_file);
        }

        for (int logId = 0; logId < LOG_NUM; logId++) {
            int fileId = logId % FILE_NUM;
            int slotId = logId / FILE_NUM;
            keyValueLogs[logId] = new KeyValueLog(path, logId,
                                                    kvFiles[fileId]->getValueFd(),
                                                    slotId * VALUE_LOG_SIZE,
                                                    kvFiles[fileId]->getBlockBuffer() + slotId * BLOCK_SIZE,
                                                    kvFiles[fileId]->getKeyBuffer() + slotId * NUM_PER_SLOT);
        }
    }
    printf("Open database complete. time spent is %lims\n", (now() - start).count());
}

PEngine::~PEngine(){
    printf("deleting engine, total life is %lims\n", (now() - start).count());

    std::thread t[RECOVER_THREAD];
    for (int i = 0; i < RECOVER_THREAD; i++) {
        t[i] = std::thread([i, this] {
            for (int logId = i; logId < LOG_NUM; logId += RECOVER_THREAD) {
                delete keyValueLogs[logId];
            }
        });
    }

    for (auto &i : t) {
        i.join();
    }

    for (auto kvFilesi : kvFiles)
        delete kvFilesi;


    if (sortLogs != nullptr) {
        for (int i = 0; i < LOG_NUM; i++)
            delete sortLogs[i];
        free(sortLogs);

        free(sortKeysArray);
        free(sortValuesArray);

        if (valueCache != nullptr) {
            free(valueCache);
        }
    }
    printf("Finish deleting engine, total life is %lims\n", (now() - start).count());
}

void PEngine::Set(const Slice &key, const Slice &value){
    
    auto logId = getLogId(key.data());
    logMutex[logId].lock();
    keyValueLogs[logId]->putValueKey(value.data(), key.data());
    set_count++;
    logMutex[logId].unlock();
    
    if(set_count % 100 == 0){
        printf("set counts: %d\n", set_count);
    }
        
}

Status PEngine::Get(const Slice &key, string *value){
    auto logId = getLogId(key.data());
    auto index = sortLogs[logId]->find(*((u_int64_t *) key.data()));

    if (index == -1) {
        return NotFound;
    } else {
        if (logId < PREPARE_CACHE_NUM && unEnlarge) {
            value->assign(reserveCache + logId * CACHE_SIZE + (index << 12), 4096);
        } else {
            auto buffer = readBuffer.get();
            keyValueLogs[logId]->readValue(index, buffer);
            value->assign(buffer, 4096);
        }
        return Ok;
    }
}
