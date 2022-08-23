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

#pragma once


#define _16BIT555(r,g,b) ((b>>3)|((g>>3)<<5)|((r>>3)<<10))
#define _16BIT565(r,g,b) ((b>>3)|((g>>2)<<5)|((r>>3)<<11))


struct bitset {
   DWORD numBits;
   DWORD *dwords;
   DWORD numDwords;
   bitset(DWORD nBits) {
      numBits=nBits;
      numDwords = ((numBits>>5) + 1);
      dwords = (DWORD*)malloc(sizeof(DWORD) * numDwords);
      memset(dwords, 0x0, sizeof(DWORD) * numDwords);
   }
   ~bitset() {
    free(dwords);
    dwords=nullptr;
   }
   void set(DWORD bit) {
       DWORD bindex = bit >> 5;
       DWORD boffset= bit&0x1F;
       if(bindex < numDwords)
          dwords[bindex] |= (1 << boffset);
   }
   void set() {
       memset(dwords,0xFF, sizeof(DWORD) * numDwords);
   }
   void clear(DWORD bit) {
       DWORD bindex = bit >> 5;
       DWORD boffset= bit&0x1F;
       if(bindex < numDwords)
          dwords[bindex] &= ~(1 << boffset);
   }
   void clear() {
       memset(dwords,0x00, sizeof(DWORD) * numDwords);
   }
   DWORD get(DWORD bit) {
       DWORD bindex = bit >> 5;
       DWORD boffset= bit&0x1F;
       if(bindex < numDwords)
          return dwords[bindex] & (1 << boffset);
       else
          return 0;
   }

};


WORD ByteSwap16(WORD num);
DWORD ByteSwap32(DWORD num);

DWORD hash(const char *str);

unsigned char* itoha(long num);



