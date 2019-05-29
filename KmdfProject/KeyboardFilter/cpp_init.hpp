#ifndef __CPP_INIT_HPP__ 
#define __CPP_INIT_HPP__

typedef void(__cdecl *PVFV)(void);

extern "C" {
    PVFV __crtXca[];
    PVFV __crtXcz[];
}

namespace Cpp
{
    void CppInitialize();

    void CppUninitialize();
}

#endif //__CPP_INIT_HPP__