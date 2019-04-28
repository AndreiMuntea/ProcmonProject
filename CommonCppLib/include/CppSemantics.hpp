#ifndef __CPP_SEMANTICS_HPP__ 
#define __CPP_SEMANTICS_HPP__ 

namespace Cpp
{

    template<class T> struct RemoveReference { typedef T Type; };
    template<class T> struct RemoveReference<T&> { typedef T Type; };
    template<class T> struct RemoveReference<T&&> { typedef T Type; };

    template<typename T>
    constexpr T&& Forward(typename Cpp::RemoveReference<T>::Type& Parameter)
    {
        return static_cast<T&&>(Parameter);
    }

    template<typename T>
    constexpr T&& Forward(typename Cpp::RemoveReference<T>::Type&& Parameter)
    {
        return static_cast<T&&>(Parameter);
    };
}

#endif //__CPP_SEMANTICS_HPP__