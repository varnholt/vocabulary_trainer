#pragma once

#include "word.h"
#include <vector>

struct Test
{
   void load(const std::string& filename);

   void pass();
   void fail();
   bool next();

   bool _flipped = false;
   std::vector<Word> _words;
   int32_t _current = 0;
};

