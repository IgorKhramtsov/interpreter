#pragma once
#include <vector>
#include <string>
#include <memory>
#include "analyzer.h"

struct Uk
{
  int pos = 0;
  int row = 0;
  int coll = 0;

  Uk(int p, int r, int c) : pos{ p }, row{ r }, coll{ c } {}
};

class Scanner
{
  static constexpr int MAXLEN = 50;


  std::string_view content;
  std::string_view token;
  std::string_view filename;

  int curPos = 0;
  int curRow = 0;
  int curColl = 0;

  int printError(const char *);
  void move(int n = 1);
  int makeToken(int, int, int);

  bool isEnd() { return (content[curPos] == '\0'); }
  void parseToken(bool * = nullptr);


public:
  Scanner(const std::unique_ptr<char[]> &, const size_t, const std::string_view &);
  ~Scanner() = default;

  int peek();
  int next();

  const std::string_view getToken() { return this->token; };
  const std::string_view getFilename() { return this->filename.substr(this->filename.find_last_of('/') + 1); };


  void getUk(int &pos, int &row, int &col)
  {
    pos = this->curPos;
    row = this->curRow;
    col = this->curColl;
  };
  void setUk(int pos, int row, int col)
  {
    this->curPos = pos;
    this->curRow = row;
    this->curColl = col;
  };

  Uk getUk()
  {
    return Uk(this->curPos, this->curRow, this->curColl);
  };
  void setUk(Uk uk)
  {
    this->curPos = uk.pos;
    this->curRow = uk.row;
    this->curColl = uk.coll;
  };
};
