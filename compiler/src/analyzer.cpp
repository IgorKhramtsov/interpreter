#include "analazyer.h"
#include <cstdio>
#include <iostream>

void Analyzer::printErr(const char *err)
{
  int pos, coll, row;
  this->m_Scanner->getUk(pos, row, coll);
  std::cout << "[Analyzer] " << this->m_Scanner->getFilename() << ":" << row + 1 << ":" << coll << " Error " << err << '\n';
  
  throw "";
  //return types::ERROR;
}

void Analyzer::addFunction(int, const char *)
{
}

void Analyzer::addArr(int, const char *, int, int)
{
}
