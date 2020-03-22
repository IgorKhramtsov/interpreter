#include "analazyer.h"
#include <cstdio>

Analyzer::Analyzer(Scanner *scanner_)
{
  this->scanner = scanner_;
}

void Analyzer::printErr(const char *err)
{
  int pos, coll, row;
  this->scanner->getUk(pos, row, coll);
  printf("[Analyzer] %s:%d:%d Error: %s\n", this->scanner->getFilename(), row + 1, coll, err);
  
  throw "";
  //return types::ERROR;
}