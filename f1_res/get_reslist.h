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


struct RESdata {
      DWORD width;
      DWORD height;
};

class RESlist {
      DWORD minWidth;
      DWORD minHeight;
      DWORD coloursCurrent;
      DWORD frequencyCurrent;
      size_t numRes;
      RESdata **list;
      public:
      RESlist() {
         minWidth=640;
         minHeight=480;
         numRes  = 0;
         list  = nullptr;
      }
      ~RESlist() {
          for(DWORD i=0; i<numRes; i++) {
              free(list[i]);
              list[i]=nullptr;
          }
          if(list)
             free(list);
          list  = nullptr;
          numRes  = 0;
      }
      size_t size() {return numRes;};
      DWORD find(DWORD width, DWORD height);
      RESdata *at(DWORD p) { if(p>=0 && p<numRes)return list[p];else return nullptr;};
      void push_back(RESdata *resData);
      bool Fill(DWORD colours, DWORD frequency, DWORD min_width, DWORD min_height, bool isInverted);
      bool Empty();
      bool Sort(bool isDescending);
      bool CheckParameters(DWORD colours, DWORD frequency, DWORD min_width, DWORD min_height);
};


class FREQlist {
      size_t num;
      DWORD *list;
      public:
      FREQlist() {
         num = 0;
         list = nullptr;
      }
      ~FREQlist() {
          if(list)
             free(list);
          num = 0;
          list = nullptr;
      }
      size_t size() {return num;};
      DWORD at(DWORD p) { if(p>=0 && p<num)return list[p];else return 0;};
      void push_back(DWORD freq);
      bool Fill(DWORD width, DWORD height, DWORD colours);
      bool Empty();
      bool Sort(bool isDescending);
      DWORD *GetFreqList(){return list;};
};

