#include "auto_alloc.h"

#include <cstdio>

AutoAlloc::AutoAlloc()
{
      m_begin = m_end = (char*)HeaderSize;
    m_destroyChain = NULL;
}

void* AutoAlloc::Alloc(size_t cb)
{
    if(m_end - m_begin < cb)
    {
        if (cb >= BlockSize)
        {
                _MemBlock* pHeader = _ChainHeader();
                _MemBlock* pNew = (_MemBlock*)malloc(HeaderSize + cb);
                if (pHeader)
                {
                    pNew->pPrev = pHeader->pPrev;
                    pHeader->pPrev = pNew;
                }
                else
                {
                    m_end = m_begin = pNew->buffer;
                    pNew->pPrev = NULL;
                }
                return pNew->buffer;        
        }
        else
        {
            _MemBlock* pNew = (_MemBlock*)malloc(sizeof(_MemBlock));
            pNew->pPrev = _ChainHeader();
            m_begin = pNew->buffer;
            m_end = m_begin + BlockSize;
        }
    }
    return m_end -= cb;
}

void AutoAlloc::Clear()
{

    // destroy
    printf("destroy\n");
    while (m_destroyChain)
    {
        m_destroyChain->fnDestroy(m_destroyChain + 1);
        m_destroyChain = m_destroyChain->pPrev;
    }
    
    // free memory
    printf("free memory\n");
    _MemBlock* pHeader = _ChainHeader();
    while (pHeader)
    {
        _MemBlock* pTemp = pHeader->pPrev;
        free(pHeader);
        pHeader = pTemp;
    }
    m_begin = m_end = (char*)HeaderSize;
}

void* AutoAlloc::Alloc(size_t cb, FnDestructor fn)
{
    _DestroyNode* pNode = (_DestroyNode*)Alloc(sizeof(_DestroyNode) + cb);
    pNode->fnDestroy = fn;
    pNode->pPrev = m_destroyChain;
    m_destroyChain = pNode;
    return pNode + 1;
}

AutoAlloc::~AutoAlloc()
{
    Clear();
}