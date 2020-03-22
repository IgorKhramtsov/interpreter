#include "scanner.h"

class Analyzer
{

  Scanner *scanner;

  void printErr(const char *);

public:
  Analyzer(Scanner *);
};