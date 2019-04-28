#ifndef __CPP_DEFAULT_DELETER_HPP__ 
#define __CPP_DEFAULT_DELETER_HPP__

namespace Cpp
{
    template<class T>
    struct DefaultDeleter
    {
        void operator()(T* Block) { delete Block; }
    };
}

#endif //__CPP_DEFAULT_DELETER_HPP__