#include "Driver.h"


//
// Implementation
//

VOID InitByteArray(IN OUT PBYTE_ARRAY Array)
{
    Array->logicalLength = 0;
    Array->head = Array->tail = NULL;
}

VOID AppendElementsByteArray(IN PBYTE_ARRAY Array, IN PVOID Elements, IN ULONG NumElements)
{
    PBYTE_ARRAY_ELEMENT node = malloc(sizeof(BYTE_ARRAY_ELEMENT));
    
    node->Length = NumElements;
    node->Data = malloc(NumElements);

    RtlCopyMemory(node->Data, Elements, NumElements);

    if (Array->logicalLength == 0) {
        Array->head = Array->tail = node;
    }
    else {
        Array->tail->next = node;
        Array->tail = node;
    }

    Array->logicalLength++;
}

VOID GetElementsByteArray(IN PBYTE_ARRAY Array, IN ULONG Index, OUT PVOID *Elements, OUT PULONG NumElements)
{
    PBYTE_ARRAY_ELEMENT node = Array->head;

    for (size_t i = 0; i < Index && node != NULL; i++)
    {
        node = node->next;
    }

    *Elements = node->Data;
    *NumElements = node->Length;
}

NTSTATUS FreeByteArray(IN PBYTE_ARRAY Array)
{
    UNREFERENCED_PARAMETER(Array);

    return STATUS_SUCCESS;
}
