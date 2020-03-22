#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <cstdio>
#include <thread>


#include "scanner.h"
#include "parser.h"
#include "defs.h"

void loadAndScan(std::string &filename);

auto main(int argc, char *argv[]) -> int
{
  setlocale(0, "rus");

  std::string filename;

  if (argc > 1) {
    filename = argv[1];// NOLINT
  } else {
    std::cout << "Enter filename: ";
    std::cin >> filename;
  }

  int key;
  do {
    loadAndScan(filename);
    key = std::cin.get();
    printf("%d",key);
  } while (key == 10 || key == 13);

  system("pause");
  return 0;
}

void loadAndScan(std::string &filename)
{
  printf("Scanning file {}\n", filename.c_str());

  std::ifstream file;
  file.open(filename.c_str(), std::ifstream::in);

  if (!file.is_open()) {
    printf("Cant open file %s\n", filename.c_str());
    exit(1);
  }

  file.seekg(0, std::ios::end);
  size_t length = file.tellg();
  printf("File`s length: %d\n", length);
  file.seekg(0, std::ios::beg);

  auto buffer = std::make_unique<char[]>(length + 1);

  file.read(buffer.get(), length);

  file.close();

  buffer.get()[length] = '\0';


  Scanner scanner(buffer, filename); // Syntax scanner
  Parser parser(&scanner); // Syntax parser
  try {
    parser.s();
  } catch (...) {
      printf("Exception has been throwed.\n");
    return;
  }
  printf("Everything nice ;)\n");
}
