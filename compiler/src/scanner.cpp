#include "scanner.h"
#include <iostream>
#include "defs.h"
#include <cstdio>


#define SKIP_CR


inline bool isLetter(char ch)
{
  if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) return true;
  return false;
}
inline bool isDigit(char ch)
{
  if (ch >= '0' && ch <= '9') return true;
  return false;
}

Scanner::Scanner(const std::unique_ptr<char[]> &buffer, const size_t count, const std::string_view &fname)
{
  this->content = std::string_view(buffer.get(), count);
  this->filename = fname;

  this->curPos = this->curRow = this->curColl = 0;
  std::cout << "Running scanner...\n";
}


int Scanner::next()
{
  this->token = "";

  while (true) {
    // skip stuff
    switch (content[curPos]) {
    case '\r':// caret return (newline part)
#ifdef SKIP_CR
      this->move();
      this->curColl -= 1;
      continue;
#endif
    case '\n':// new line
    case ' ':// space
    case '\t':// tab
      this->move();
      continue;
    case '/':
      if (content[curPos + 1] == '/') {
        while (!isEnd() && content[curPos] != '\n') this->move();
      }
      if (content[curPos + 1] == '*') {
        while (!isEnd() && !(content[curPos] == '*' && content[curPos + 1] == '/')) this->move();
        this->move(2);
      }
      continue;
    case '\0':// eof
      return types::END;
    }
    if (curPos >= this->content.length()) {// no eof
      return this->printError("Cant find end of file");
    }

    break;
  }

  int type = peek();
  move(token.length());
  return type;
}


int Scanner::peek()
{
  if (isLetter(content[curPos])) {

    try {
      this->parseToken();
    } catch (const char *err) {
      return printError(err);
    }

    if (this->token == "void") {
      return types::VOID;
    } else if (this->token == "int") {
      return types::INT;
    } else if (this->token == "bool") {
      return types::BOOL;
    } else if (this->token == "if") {
      return types::IF;
    } else if (this->token == "true" || this->token == "false") {
      return types::BOOL_CONST;
    } else if (this->token == "return") {
      return types::RETURN;
    } else {
      return types::ID;
    }

  } else if (isDigit(content[curPos])) {

    bool isAllDigits;
    try {
      this->parseToken(&isAllDigits);
    } catch (const char *err) {
      return printError(err);
    }

    if (!isAllDigits) {
      return printError("Unrecognized token");
    } else {
      return types::INT_CONST;
    }

  } else if (content[curPos] == '<') {
    if (content[curPos + 1] == '=') {
      return makeToken(curPos, 2, types::LEQ);
    }
    return makeToken(curPos, 1, types::LESS);
  } else if (content[curPos] == '>') {
    if (content[curPos + 1] == '=') {
      return makeToken(curPos, 2, types::MEQ);
    }
    return makeToken(curPos, 1, types::MORE);
  } else if (content[curPos] == '=') {
    if (content[curPos + 1] == '=')
      return makeToken(curPos, 2, types::EQ);
    return makeToken(curPos, 1, types::ASSIGN);
  } else if (content[curPos] == '!') {
    if (content[curPos + 1] == '=') {
      return makeToken(curPos, 2, types::NEQ);
    } else {
      return makeToken(curPos, 1, printError("Unrecognized token"));
    }
  } else if (content[curPos] == '+') {
    if (content[curPos + 1] == '=') {
      return makeToken(curPos, 2, types::SUMEQ);
    } else if (content[curPos + 1] == '+') {
      return makeToken(curPos, 2, types::INC);
    }
    return makeToken(curPos, 1, types::SUM);
  } else if (content[curPos] == '-') {
    if (content[curPos + 1] == '=') {
      return makeToken(curPos, 2, types::SUBEQ);
    } else if (content[curPos + 1] == '-') {
      return makeToken(curPos, 2, types::DEC);
    }
    return makeToken(curPos, 1, types::SUB);
  } else if (content[curPos] == '*') {
    if (content[curPos + 1] == '=') {
      return makeToken(curPos, 2, types::MULEQ);
    }
    return makeToken(curPos, 1, types::MUL);
  } else if (content[curPos] == '%') {
    if (content[curPos + 1] == '=') {
      return makeToken(curPos, 2, types::MODEQ);
    }
    return makeToken(curPos, 1, types::MOD);
  } else if (content[curPos] == '/') {
    if (content[curPos + 1] == '=') {
      return makeToken(curPos, 2, types::DIVEQ);
    }
    return makeToken(curPos, 1, types::DIV);
  } else if (content[curPos] == ';') {
    return makeToken(curPos, 1, types::SEMI);
  } else if (content[curPos] == ',') {
    return makeToken(curPos, 1, types::COMA);
  } else if (content[curPos] == '(') {
    return makeToken(curPos, 1, types::LBKT);
  } else if (content[curPos] == ')') {
    return makeToken(curPos, 1, types::RBKT);
  } else if (content[curPos] == '{') {
    return makeToken(curPos, 1, types::fLBKT);
  } else if (content[curPos] == '}') {
    return makeToken(curPos, 1, types::fRBKT);
  } else if (content[curPos] == '[') {
    return makeToken(curPos, 1, types::sLBKT);
  } else if (content[curPos] == ']') {
    return makeToken(curPos, 1, types::sRBKT);
  }

  return makeToken(curPos, 1, printError("Unsopported case"));
}


int Scanner::makeToken(int start, int count, int type)
{
  this->token = content.substr(start, count);
  return type;
}

void Scanner::move(int n)
{
  for (int i = 0; i < n; i++) {
    this->curPos++;
    this->curColl++;
    if (content[curPos] == '\n') {
      this->curRow++;
      this->curColl = 0;
    }
  }
}

void Scanner::parseToken(bool *isAllDigits)
{
  int i = 0;
  if (isAllDigits != nullptr) *isAllDigits = true;
  while (i < this->MAXLEN) {
    if (isLetter(content[curPos + i])) {
      i++;
      if (isAllDigits != nullptr) *isAllDigits = false;
      continue;
    } else if (isDigit(content[curPos + i])) {
      i++;
      continue;
    } else {
      break;
    }
  }
  if (i >= this->MAXLEN)
    throw("Exceeded max token length");

  this->token = content.substr(curPos, i);
}

int Scanner::printError(const char *err)
{
  std::cout << "[Scanner] " << this->filename << ":" << this->curRow + 1 << ":" << this->curColl << " Error " << err << '\n';
  return types::ERROR;
}
