#ifndef AUTO_ALLOC_H_
#define AUTO_ALLOC_H_

#include <cstdlib>    /* malloc, free, rand */
#include <new>


typedef void (*FnDestructor)(void* pThis);
 
/*
class AutoAlloc
{
public:
    ~AutoAlloc();                                  // 析构函数。自动调用Clear释放内存
    void* allocate(size_t cb);                     // 类似于malloc(cb)
    void* allocate(size_t cb, FnDestructor fn);    // 申请内存并指定析构函数
    void clear();                                  // 析构并释放所有分配的对象
};
*/

class AutoAlloc
{
public:
    enum { BlockSize = 2048 };
private:
    struct _MemBlock
    {
        _MemBlock* pPrev;
        char buffer[BlockSize];
    };
    enum { HeaderSize = sizeof(_MemBlock) - BlockSize };
 
    char* m_begin;
    char* m_end;

    _MemBlock* _ChainHeader() const
    {
        return (_MemBlock*)(m_begin - HeaderSize);
    }

    struct _DestroyNode
    {
        _DestroyNode* pPrev;
        FnDestructor fnDestroy;
    };
    _DestroyNode* m_destroyChain;

public:
    AutoAlloc();
    ~AutoAlloc();                                  // 析构函数。自动调用Clear释放内存
    void* Alloc(size_t cb);                     // 类似于malloc(cb)
    void* Alloc(size_t cb, FnDestructor fn);    // 申请内存并指定析构函数
    void Clear();                                  // 析构并释放所有分配的对象
    
};

template <class Type>
struct DestructorTraits
{
    static void Destruct(void* pThis)
    {
        ((Type*)pThis)->~Type();
    }
};

// 类似于new Type
template <class Type, class AllocType>
inline Type* New(AllocType& alloc)
{
    void* obj = alloc.Alloc(sizeof(Type), DestructorTraits<Type>::Destruct);
    return new(obj) Type;
}

// 类似于new Type(arg1)
template <class Type, class ArgType1, class AllocType>
Type* New(ArgType1 arg1, AllocType& alloc)
{
    void* obj = alloc.Alloc(sizeof(Type), DestructorTraits<Type>::Destruct);
    return new(obj) Type(arg1);
}

// 类似于new Type[count]
template <class Type, class AllocType>
Type* NewArray(size_t count, AllocType& alloc)
{
    void* obj = alloc.Alloc(sizeof(Type)*count, DestructorTraits<Type>::Destruct);
    return new(obj) Type[count];
}

#endif /* AUTO_ALLOC_H_ */