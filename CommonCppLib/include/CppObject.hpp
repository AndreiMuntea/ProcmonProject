#ifndef __CPP_OBJECT_HPP__ 
#define __CPP_OBJECT_HPP__

namespace Cpp
{
    class Object
    {
    public:
        Object() = default;
        virtual ~Object() = default;

        inline void Validate() { isValid = true; }
        inline void Invalidate() { isValid = false; }
        inline bool IsValid() const { return isValid; }

    private:
        bool isValid = false;
    };
}

#endif //__CPP_OBJECT_HPP__