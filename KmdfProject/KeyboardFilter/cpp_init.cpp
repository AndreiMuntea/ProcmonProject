#include "cpp_init.hpp"
#include "cpp_definitions.hpp"


#define _CRTALLOC(x) __declspec(allocate(x))

#pragma section(".CRT$XCA", long, read)
#pragma section(".CRT$XCZ", long, read)

_CRTALLOC(".CRT$XCA") PVFV __crtXca[] = { nullptr };
_CRTALLOC(".CRT$XCZ") PVFV __crtXcz[] = { nullptr };

#pragma comment(linker, "/merge:.CRT=.rdata")

typedef struct _CPP_DESTRUCTOR
{
    LIST_ENTRY Entry;
    PVFV       Destructor;
}CPP_DESTRUCTOR, *PCPP_DESTRUCTOR;

static CPP_DESTRUCTOR gDestructorsList;

int
CppRegisterDestructor(
    _In_ PVFV Destructor
)
{
    auto entry = (PCPP_DESTRUCTOR)ExAllocatePoolWithTag(NonPagedPool, sizeof(CPP_DESTRUCTOR), CPP_TAG_CPP);
    if (!entry)
    {
        NT_ASSERT(false);
        return -1;
    }

    entry->Destructor = Destructor;
    InsertTailList(&gDestructorsList.Entry, &entry->Entry);

    return 0;
}

void
CppInitializeDestructorsList()
{
    InitializeListHead(&gDestructorsList.Entry);
    gDestructorsList.Destructor = nullptr;
}

void
CppCallStaticDestructors()
{
    while (!IsListEmpty(&gDestructorsList.Entry))
    {
        auto destructorEntry = (PCPP_DESTRUCTOR)RemoveHeadList(&gDestructorsList.Entry);
        if (destructorEntry->Destructor)
        {
            destructorEntry->Destructor();
        }
        ExFreePoolWithTag(destructorEntry, CPP_TAG_CPP);
    }
}

void
CppCallStaticConstructors(
    _In_ PVFV* FirstPVFVEntry,
    _In_ PVFV* LastPVFVEntry
)
{
    for (auto currentPVFVEntry = FirstPVFVEntry; currentPVFVEntry < LastPVFVEntry; ++currentPVFVEntry)
    {
        if (*currentPVFVEntry)
        {
            (**currentPVFVEntry)();
        }
    }
}

int __cdecl
atexit(
    _In_ PVFV Destructor
)
{
    return CppRegisterDestructor(Destructor);
}

void Cpp::CppInitialize()
{
    CppInitializeDestructorsList();
    CppCallStaticConstructors(__crtXca, __crtXcz);
}

void Cpp::CppUninitialize()
{
    CppCallStaticDestructors();
}
