//======================================================================
//  WordBoundaries.cpp - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Apr  6 09:35:57 2018
//----------------------------------------------------------------------

#include <cstring>
#include <iostream>
#include <charconv>
#include "WordBoundaries.h"
#include "fast_double_parser.h"


using namespace std;

// Build the boundaries of all space separated words in the string.
void WordBoundaries::ParseBoundaries(const char *s)
{
  clear();

  InspectionString = s;
  int p = 0;
  while (s[p])
  {
    // Find first character of word
    while(s[p] && isspace(s[p]))
      p++;

    // Find last word character
    int pe=p;
    while (s[pe] && !isspace(s[pe]))
      pe++;

    if (pe!= p)
      push_back(pair<int,int>(p,pe));
    p = pe;
  }
}

// Check if the Index' word matches the Candidate.
bool WordBoundaries::CheckMatch(int Index, const char* Candidate) const
{
  if (Index >= (int)size())
    return false;

  const pair<int,int>& p((*this)[Index]);

  if ((int)strlen(Candidate) != (int)(p.second - p.first))
      return false;

  return strncasecmp(Candidate,InspectionString+p.first,
                     p.second-p.first) == 0;
}

// Get the remaining string starting at word number Index
const char *WordBoundaries::GetRestAsString(int Index)
{
  return InspectionString + (*this)[Index].first;
}

// Get a word as a floating point value.
double WordBoundaries::GetFloat(int Index)
{
  const char *p = InspectionString;
  const pair<int,int>& pp = (*this)[Index];
  double x;
  fast_double_parser::parse_number(p+pp.first, &x);
  return x;
}

// Get a word as a floating point value.
int WordBoundaries::GetInt(int Index)
{
  const pair<int,int>& pp = (*this)[Index];
  int i;
  from_chars(InspectionString+pp.first, InspectionString+pp.second, i);

  return i;
}

// Get a word as a new string
string WordBoundaries::GetWordAsString(int Index) const
{
    auto p = (*this)[Index];
    return string(InspectionString + p.first,
                  InspectionString + p.second);
}

std::ostream& operator<<(std::ostream& os, const WordBoundaries&b)
{
  for (auto p: b)
    os << "[" << p.first << ',' << p.second << ") ";
  return os;
}

