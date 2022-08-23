/*
The MIT License (MIT)
Copyright © 2022 Matt Wells

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the “Software”), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "pch.h"

#include "convert.h"

//_________________________
DWORD hash(const char* str) {

	DWORD hash = 0;
	int c;

	while ((c = *str++))
	{
		/* hash = hash * 33 ^ c */
		hash = ((hash << 5) + hash) ^ c;
	}

	return hash;
}


//________________________
WORD ByteSwap16(WORD num) {

	return (((num >> 8)) | (num << 8));
}


//_________________________
DWORD ByteSwap32(DWORD num) {

	return (((num & 0x000000FF) << 24) + ((num & 0x0000FF00) << 8) +
		((num & 0x00FF0000) >> 8) + ((num & 0xFF000000) >> 24));
}


//____________________________
unsigned char* itoha(long num) {

	unsigned char* byte_string = new unsigned char[5];

	byte_string[0] = (BYTE)(num & 0x000000FF);
	byte_string[1] = (BYTE)((num & 0x0000FF00) >> 8);
	byte_string[2] = (BYTE)((num & 0x00FF0000) >> 16);
	byte_string[3] = (BYTE)((num & 0xFF000000) >> 24);
	byte_string[4] = '\0';

	return byte_string;
}
