#ifndef __QUEUE_HPP__ 
#define __QUEUE_HPP__

#include "LinkedList.hpp"

namespace Cpp
{
    template <class T>
    class Queue : public CppNonPagedObject<'TDQ#'>
    {
    public:
        Queue();
        virtual ~Queue();

        void Push(_In_ __drv_aliasesMem T* Element);
        T* Pop();

        bool IsEmpty() const;
        size_t GetNoElements() const;

    private:
        LinkedList<T> list;
    };

    template<class T>
    inline Queue<T>::Queue()
    {
        Validate();
    }

    template<class T>
    inline Queue<T>::~Queue()
    {
        list.Flush();
    }

    template<class T>
    inline void Queue<T>::Push(_In_ __drv_aliasesMem T* Element)
    {
        list.InsertTail(Element);
    }

    template<class T>
    inline T * Queue<T>::Pop()
    {
        return list.IsListEmpty() ? (T*)nullptr
            : (T*)list.RemoveHead();
    }

    template<class T>
    inline bool Queue<T>::IsEmpty() const
    {
        return list.IsListEmpty();
    }

    template<class T>
    inline size_t Queue<T>::GetNoElements() const
    {
        return list.GetNoElements();
    }
}

#endif //__QUEUE_HPP__
