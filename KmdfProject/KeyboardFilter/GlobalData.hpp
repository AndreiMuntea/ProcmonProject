#ifndef __GLOBAL_DATA_HPP__ 
#define __GLOBAL_DATA_HPP__

#include "File.hpp"
#include "ThreadPool.hpp"

#include "cpp_unique_pointer.hpp"

typedef struct _GLOBAL_DATA
{
    Cpp::UniquePointer<Cpp::ThreadPool> ThreadPool;
    Cpp::UniquePointer<Cpp::File> File;
}GLOBAL_DATA, *PGLOBAL_DATA;

extern GLOBAL_DATA gDrvData;


#endif //__GLOBAL_DATA_HPP__