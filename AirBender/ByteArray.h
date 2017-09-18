#pragma once

#define INITIAL_ARRAY_CAPACITY PAGE_SIZE
#define ARRAY_POOL_TAG	'arrA'

//
// Some insane macro-magic =3
// 
#define P99_PROTECT(...) __VA_ARGS__
#define APPEND_BYTE_ARRAY(_arr_, _bytes_)   do {BYTE b[] = _bytes_; \
                                            AppendElementsByteArray(&_arr_, b, _countof(b)); } while (0)

typedef struct _BYTE_ARRAY_ELEMENT
{
    LPVOID Data;

    ULONG Length;

    struct _BYTE_ARRAY_ELEMENT *next;

} BYTE_ARRAY_ELEMENT, *PBYTE_ARRAY_ELEMENT;

typedef struct _BYTE_ARRAY
{
    ULONG logicalLength;

    PBYTE_ARRAY_ELEMENT head;

    PBYTE_ARRAY_ELEMENT tail;

} BYTE_ARRAY, *PBYTE_ARRAY;

VOID InitByteArray(IN OUT PBYTE_ARRAY Array);

VOID AppendElementsByteArray(IN PBYTE_ARRAY Array, IN PVOID Elements, IN ULONG NumElements);

VOID GetElementsByteArray(IN PBYTE_ARRAY Array, IN ULONG Index, OUT PVOID *Elements, OUT PULONG NumElements);

NTSTATUS FreeByteArray(IN PBYTE_ARRAY Array);