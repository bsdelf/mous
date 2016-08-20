#pragma once

#include <mutex>

#include "SemVar.hpp"

namespace scx {

template <typename item_t>
class LPVBuffer
{
public:
    LPVBuffer():
        m_FreeListSemVar(0),
        m_DataListSemVar(0),
        m_BufferCount(0),
        m_BufferArray(nullptr),
        m_FreeIndex(-1),
        m_DataIndex(-1)
    {
    }

    ~LPVBuffer()
    {
        m_FreeListSemVar.Clear();
        m_DataListSemVar.Clear();
        if (m_BufferArray != nullptr)
            delete[] m_BufferArray;
    }

    void AllocBuffer(size_t bufCount)
    {
        m_BufferCount = bufCount;
        m_BufferArray = new item_t[bufCount];

        ResetPV();
    }

    void ClearBuffer()
    {
        m_FreeListSemVar.Clear();
        m_DataListSemVar.Clear();

        delete[] m_BufferArray;
        m_BufferCount = 0;
        m_BufferArray = nullptr;

        m_FreeIndex = -1;
        m_DataIndex = -1;
    }

    /**
     * This method should be called after both customer and producer
     * has been suspended by TakeFree()/TakeData().
     * After calling this method, 
     * the producer will begin to work again, then the customer.
     */
    void ResetPV()
    {
        m_DataListSemVar.Clear();
        m_FreeListSemVar.Clear();

        for (size_t i = 0; i < m_BufferCount; ++i) {
            m_FreeListSemVar.Post();
        }

        m_FreeIndex = -1;
        m_DataIndex = -1;
    }

    size_t BufferCount() const
    {
        return m_BufferCount;
    }

    size_t FreeCount() const
    {
        return m_FreeListSemVar.Value();
    }

    size_t DataCount() const
    {
        return m_DataListSemVar.Value();
    }

    /**
     * This method can be used for initialize buffer.
     */
    item_t* RawItemAt(size_t i)
    {
        return m_BufferArray+i;
    }

    item_t* TakeFree()
    {
        m_FreeListSemVar.Wait(); 
        m_FreeIndex = (m_FreeIndex+1) % m_BufferCount;
        return m_BufferArray + m_FreeIndex;
    }

    item_t* TakeFreeS()
    {
        m_FreeListSemVar.Wait(); 
        m_FreeListMutex.lock();
        m_FreeIndex = (m_FreeIndex+1) % m_BufferCount;
        item_t* item = m_BufferArray + m_FreeIndex;
        m_FreeListMutex.unlock();
        return item;
    }

    void RecycleFree(item_t* val = nullptr)
    {
        m_DataListSemVar.Post();
    }

    item_t* TakeData()
    {
        m_DataListSemVar.Wait(); 
        m_DataIndex = (m_DataIndex+1) % m_BufferCount;
        return m_BufferArray + m_DataIndex;
    }

    item_t* TakeDataS()
    {
        m_DataListSemVar.Wait(); 
        m_DataListMutex.lock();
        m_DataIndex = (m_DataIndex+1) % m_BufferCount;
        item_t* item = m_BufferArray + m_DataIndex;
        m_DataListMutex.unlock();
        return item;
    }

    void RecycleData(item_t* p = nullptr)
    {
        m_FreeListSemVar.Post();
    }

    /**
     * Clear the empty-buffer, so that the producer can be blocked.
     */
    void ClearFree()
    {
        m_FreeListSemVar.Clear();
    }

    /**
     * Clear the filled-data, so that the customer can be blocked.
     */
    void ClearData()
    {
        m_DataListSemVar.Clear();
    }

private:
    SemVar m_FreeListSemVar;
    SemVar m_DataListSemVar;
    std::mutex m_FreeListMutex;
    std::mutex m_DataListMutex;

    size_t m_BufferCount;
    item_t* m_BufferArray;

    size_t m_FreeIndex;
    size_t m_DataIndex;
};

}

