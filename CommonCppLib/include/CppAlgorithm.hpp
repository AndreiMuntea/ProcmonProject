#ifndef __CPP_ALGORITHM_HPP__ 
#define __CPP_ALGORITHM_HPP__

namespace Cpp
{
    template <class T>
    T Min(const T& Left, const T& Right)
    {
        return (Left < Right) ? Left 
                              : Right;
    }

    template <class T>
    T Max(const T& Left, const T& Right)
    {
        return (Left > Right) ? Left
                              : Right;
    }

    template <class T>
    void Swap(T& Left, T& Right)
    {
        T temp = Left;
        Left = Right;
        Right = temp;
    }
}

#endif //__CPP_ALGORITHM_HPP__