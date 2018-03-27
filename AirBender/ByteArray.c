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
	if (node != NULL) {
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
}

VOID GetElementsByteArray(IN PBYTE_ARRAY Array, IN ULONG Index, OUT PVOID *Elements, OUT PULONG NumElements)
{
	PBYTE_ARRAY_ELEMENT node = Array->head;
	if (node != NULL) {
		for (size_t i = 0; i < Index; i++)
		{
			node = node->next;
		}

		*Elements = node->Data;
		*NumElements = node->Length;
	}
}

VOID FreeByteArray(IN PBYTE_ARRAY Array)
{
    PBYTE_ARRAY_ELEMENT node = Array->head;

    while (node != NULL)
    {
        free(node->Data);
        node = node->next;
    }

    RtlZeroMemory(Array, sizeof(BYTE_ARRAY));
}
