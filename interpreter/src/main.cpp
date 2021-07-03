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
#include "FileWatcher.h"

void loadAndScan(std::string &filename);

#define disable_catch 0

auto main(int argc, char *argv[]) -> int
{
  std::string filename;

  if (argc > 1) {
    filename = argv[1];// NOLINT
  } else {
    std::cout << "Enter filename: ";
    std::cin >> filename;
  }

  int key;
  loadAndScan(filename);
  FileWatcher watcher(filename, 350);
  watcher.start([&filename]() -> void { 
    system("CLS");
    loadAndScan(filename); 
  });

  system("pause");
  return 0;
}

void loadAndScan(std::string &filename)
{
  std::cout << "Scanning file " << filename.c_str() << '\n';

  std::ifstream file;
  file.open(filename.c_str(), std::ios::binary);

  if (!file.is_open()) {
    std::cout << "Cant open file " << filename.c_str() << '\n';
    exit(1);
  }

  file.seekg(0, std::ios::end);
  size_t length = file.tellg();
  std::cout << "File`s length: " << length << '\n';
  file.seekg(0, std::ios::beg);

  auto buffer = std::make_unique<char[]>(length + 1);
  file.read(buffer.get(), length);
  file.close();

  buffer.get()[length] = '\0';

  Scanner scanner(buffer, length + 1, filename);// Syntax scanner
  Parser parser(&scanner);// Syntax parser

  try {
    parser.s();
    parser.show();
    parser.startInterp();
#if disable_catch
  } catch (...) {
    throw;
#else
  } catch (const char* exc) {
    std::cout << exc;
    return;
#endif
  }

  std::cout << "Everything nice ;)\n";
}
