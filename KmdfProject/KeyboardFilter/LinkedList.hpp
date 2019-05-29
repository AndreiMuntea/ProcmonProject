#ifndef __LINKED_LIST_HPP__ 
#define __LINKED_LIST_HPP__

#include "LinkedListEntry.hpp"

namespace Cpp
{
    template <class T>
    class LinkedList : public LinkedListEntry
    {
    public:
        LinkedList();
        virtual ~LinkedList();

        void InsertTail(_In_ __drv_aliasesMem LinkedListEntry* Entry);
        void InsertHead(_In_ __drv_aliasesMem LinkedListEntry* Entry);

        LinkedListEntry* RemoveTail();
        LinkedListEntry* RemoveHead();

        void RemoveEntry(_Inout_ LinkedListEntry* Entry);
        void Flush();

        bool IsListEmpty() const;
        size_t GetNoElements() const;

        class Iterator
        {
        public:
            Iterator(LinkedListEntry* Element) : current{ Element } {};
            ~Iterator() { current = nullptr; }

            Iterator& operator++() { current = current->flink; return *this; }
            T* GetRawPointer() { return (T*)current; }

            bool operator==(const Iterator& Other) const { return current == Other.current; }
            bool operator!=(const Iterator& Other) const { return current != Other.current; }

        private:
            LinkedListEntry* current;
        };

        Iterator begin() { return Iterator{ this->flink }; }
        Iterator end() { return Iterator{ this }; }

    private:
        void CheckListEntry(_In_ LinkedListEntry* Entry) const;
        void FastFailOnCorruption(_In_ LinkedListEntry* Entry) const;

        size_t noElements = 0;
    };

    template<class T>
    inline LinkedList<T>::LinkedList()
    {
        Validate();
    }

    template<class T>
    inline LinkedList<T>::~LinkedList()
    {
        Flush();
    }

    template<class T>
    inline void LinkedList<T>::InsertTail(_In_ __drv_aliasesMem LinkedListEntry* Entry)
    {
        FastFailOnCorruption(Entry);

        Entry->blink = this->blink;
        Entry->flink = this;

        this->blink->flink = Entry;
        this->blink = Entry;

        ++noElements;
    }

    template<class T>
    inline void LinkedList<T>::InsertHead(_In_ __drv_aliasesMem LinkedListEntry* Entry)
    {
        FastFailOnCorruption(Entry);

        Entry->flink = this->flink;
        Entry->blink = this;

        this->flink->blink = Entry;
        this->flink = Entry;

        ++noElements;
    }

    template<class T>
    inline LinkedListEntry* LinkedList<T>::RemoveTail()
    {
        auto tail = this->blink;
        RemoveEntry(tail);
        return tail;
    }

    template<class T>
    inline LinkedListEntry* LinkedList<T>::RemoveHead()
    {
        auto head = this->flink;
        RemoveEntry(head);
        return head;
    }

    template<class T>
    inline void LinkedList<T>::RemoveEntry(_Inout_ LinkedListEntry* Entry)
    {
        FastFailOnCorruption(Entry);

        auto next = Entry->flink;
        auto prev = Entry->blink;

        next->blink = prev;
        prev->flink = next;

        --noElements;
    }

    template<class T>
    inline void LinkedList<T>::Flush()
    {
        while (!IsListEmpty())
        {
            auto head = RemoveHead();
            delete head;
        }
    }

    template<class T>
    inline bool LinkedList<T>::IsListEmpty() const
    {
        return noElements == 0;
    }

    template<class T>
    inline size_t LinkedList<T>::GetNoElements() const
    {
        return noElements;
    }

    template<class T>
    inline void LinkedList<T>::CheckListEntry(_In_ LinkedListEntry* Entry) const
    {
        UNREFERENCED_PARAMETER(Entry);

        NT_ASSERT(Entry->flink->blink == Entry);
        NT_ASSERT(Entry->blink->flink == Entry);
    }

    template<class T>
    inline void LinkedList<T>::FastFailOnCorruption(_In_ LinkedListEntry* Entry) const
    {
        CheckListEntry(Entry);
        CheckListEntry(this->flink);
        CheckListEntry(this->blink);
    }
}

#endif //__LINKED_LIST_HPP__
