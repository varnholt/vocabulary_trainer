#include "test.h"


#include <fstream>
#include <sstream>

void Test::load(const std::string& filename)
{
   _words.clear();
   _current = 0;

   std::ifstream file(filename);
   std::vector<std::vector<std::string>> data;
   std::string line = "";

   std::string delimiter = "";

   while (getline(file, line))
   {
      if (delimiter.empty())
      {
         if (line.find(";") != std::string::npos)
         {
            delimiter = ";";
         }
         else if (line.find(",") != std::string::npos)
         {
            delimiter = ",";
         }
         else if (line.find("|") != std::string::npos)
         {
            delimiter = "|";
         }
      }

      std::vector<std::string> vec;

      std::istringstream s(line);
      std::string field;
      while (std::getline(s, field, delimiter[0]))
      {
         if (!field.empty())
         {
            vec.push_back(field);
         }
      }
      data.push_back(vec);
   }

   for (const auto& d : data)
   {
      if (d.empty())
      {
         continue;
      }

      Word w;
      std::string l1;
      std::string l2;

      for (auto i = 0u; i < d.size() - 1; i++)
      {
         l1.append(d[i]);
         l1.append("\n");
      }

      l2 = d[d.size() - 1];

      w._language_1 = _flipped ? l2 : l1;
      w._language_2 = _flipped ? l1 : l2;

      _words.push_back(w);
   }
}


void Test::pass()
{
   _words[_current]._score++;
}


void Test::fail()
{
   _words[_current]._score--;
}


bool Test::next()
{
   std::vector<int32_t> bad_ones;

   int32_t i = 0;
   for (const auto& w : _words)
   {
      if (w._score <= 0)
      {
         bad_ones.push_back(i);
      }

      i++;
   }

   if (bad_ones.empty())
   {
      return false;
   }

   // pick a random bad one, but not the same one again
   auto next = 0;
   do {
      next = bad_ones[(std::rand() % bad_ones.size())];
   }
   while (_current == next && bad_ones.size() > 1);

   _current = next;
   return true;
}
