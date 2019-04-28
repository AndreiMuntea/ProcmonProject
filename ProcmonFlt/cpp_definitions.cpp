#include "cpp_definitions.hpp"

void* __cdecl
operator new(
    _In_ size_t Size
)
{
    NT_ASSERTMSG("Global operator new. Please override this!", false);
    return ExAllocatePoolWithTag(PagedPool, Size, CPP_TAG_DEF);
}

void* __cdecl
operator new[](
    _In_ size_t Size
)
{
    NT_ASSERTMSG("Global operator new[]. Please override this!", false);
    return ExAllocatePoolWithTag(PagedPool, Size, CPP_TAG_DEF);
}

void __cdecl
operator delete(
    _Inout_  PVOID Block
)
{
    NT_ASSERTMSG("Global operator delete. Please override this!", false);
    ExFreePoolWithTag(Block, CPP_TAG_DEF);
}

void __cdecl
operator delete[](
    _Inout_  PVOID Block
)
{
    NT_ASSERTMSG("Global operator delete[]. Please override this!", false);
    ExFreePoolWithTag(Block, CPP_TAG_DEF);
}

void __cdecl
operator delete(
    _Inout_  PVOID Block,
    _In_ ULONG Tag
)
{
    NT_ASSERTMSG("Global operator delete. Please override this!", false);
    ExFreePoolWithTag(Block, Tag);
}

void __cdecl
operator delete[](
    _Inout_  PVOID Block,
    _In_ ULONG Tag
)
{
    NT_ASSERTMSG("Global operator delete[]. Please override this!", false);
    ExFreePoolWithTag(Block, Tag);
}

void __cdecl
operator delete(
    _Inout_ PVOID Block,
    _In_ size_t Size
)
{
    UNREFERENCED_PARAMETER(Block);
    UNREFERENCED_PARAMETER(Size);
    NT_ASSERTMSG("Global operator delete. Please override this!", false);
}

void __cdecl
operator delete[](
    _Inout_ PVOID Block,
    _In_ size_t Size
)
{
    UNREFERENCED_PARAMETER(Block);
    UNREFERENCED_PARAMETER(Size);
    NT_ASSERTMSG("Global operator delete[]. Please override this!", false);
}

int _fltused = 0;