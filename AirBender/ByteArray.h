/*
MIT License

Copyright (c) 2017 Benjamin "Nefarius" Höglinger

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


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