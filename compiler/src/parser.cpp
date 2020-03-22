#include "parser.h"
#include "defs.h"
#include <iostream>
#include <string>
#include <cstdio>


static bool isVarType(int type) { return (type == types::INT || type == types::BOOL); }

Parser::Parser(Scanner *sc)
{
  this->scanner = sc;
  printf("Running parser...\n");
}

void Parser::s()
{
  type = scanner->next();

  if (isVarType(type)) {
    funcOrVar();
    s();
  } else if (type == types::END) {
    return;
  } else {
    printErr((std::string("Unknown token: ") + std::string(this->scanner->getToken().data())).c_str());
  }
}


void Parser::funcOrVar()
{
  int pos, col, row;// NOLINT
  scanner->getUk(pos, row, col);
  if (scanner->next() != types::ID) {
    printErr("Ожидался идентификатор");
  }
  switch (scanner->next()) {
  case types::LBKT:
    scanner->setUk(pos, row, col);
    function();
    break;
  case types::ASSIGN:
  case types::sLBKT:
  case types::SEMI:
  case types::COMA:
    scanner->setUk(pos, row, col);
    variables();
    break;
  default:
    printErr("Ожидалась переменная или функция");
  }
}

void Parser::function()
{
  type = scanner->next();
  if (type != types::ID)
    printErr("Ожидался идентификатор");
  if (scanner->next() != types::LBKT)
    printErr("Ожидалось (");
  if (scanner->next() != types::RBKT)
    printErr("Ожидалось )");
  codeBlock();
}

void Parser::variables()
{
  if (scanner->next() != types::ID)
    printErr("Ожидался идентификатор");
  switch (scanner->next()) {
  case types::sLBKT:
    expression();
    if (scanner->next() != types::sRBKT)
      printErr("Ожидалось ]");
    if (scanner->next() != types::sLBKT)
      printErr("Ожидалось [");
    expression();
    if (scanner->next() != types::sRBKT)
      printErr("Ожидалось ]");
    switch (scanner->next()) {
    case types::COMA:
      variables();
      break;
    case types::SEMI:
    default:
      break;
    }
    break;
  case types::ASSIGN:
    expression();
    type = scanner->next();
    if (type == types::COMA)
      variables();
    else if (type != types::SEMI)
      printErr("Ожидалось ;");

    break;
  case types::SEMI:
    break;
  case types::COMA:
    variables();
    break;
  }
}

void Parser::codeBlock()
{
  if (scanner->next() != types::fLBKT)
    printErr("Ожидалось {");

  int pos, col, row;
  scanner->getUk(pos, row, col);
  type = scanner->next();

  while (type == types::INT || type == types::BOOL || type == types::IF || type == types::ID || type == types::fLBKT || type == types::RETURN) {
    if (type == types::INT || type == types::BOOL)
      variables();
    else if (type == types::IF) {
      if (scanner->next() != types::LBKT)
        printErr("Ожидалось (");
      expression();
      if (scanner->next() != types::RBKT)
        printErr("Ожидалось )");
      codeBlock();
    } else if (type == types::ID) {
      switch (scanner->next()) {
      case types::LBKT:
        if (scanner->next() != types::RBKT)
          printErr("Ожидалось )");
        if (scanner->next() != types::SEMI)
          printErr("Ожидалось ;");
        break;
      case types::ASSIGN:
        expression();
        if (scanner->next() != types::SEMI)
          printErr("Ожидалось ;");
        break;
      case types::INC:
      case types::DEC:
      case types::SUMEQ:
      case types::SUBEQ:
      case types::DIVEQ:
      case types::MULEQ:
        scanner->setUk(pos, row, col);
        expression();
        if (scanner->next() != types::SEMI)
          printErr("Ожидалось ;");
        break;
      default:
        printErr("Ожидался вызов функции или присваивание");
        break;
      }
    } else if (type == types::fLBKT) {
      scanner->setUk(pos, row, col);
      codeBlock();
    } else if (type == types::RETURN) {
      expression();
      if (scanner->next() != types::SEMI)
        printErr("Ожидалось ;");
    }

    scanner->getUk(pos, row, col);
    type = scanner->next();
  }
  scanner->setUk(pos, row, col);

  if (scanner->next() != types::fRBKT)
    printErr("Ожидалось }");
}

void Parser::expression()
{
  int pos, col, row;

  element();

  scanner->getUk(pos, row, col);
  switch (scanner->next()) {
  case types::MORE:
  case types::LESS:
  case types::MEQ:
  case types::LEQ:
  case types::EQ:
    expression();
    break;

  default:
    scanner->setUk(pos, row, col);
    break;
  }
}

void Parser::element()
{
  int pos, col, row;
  scanner->getUk(pos, row, col);
  if (scanner->next() == types::LBKT) {
    element();
    if (scanner->next() != types::RBKT)
      printErr("Ожидалось )");
    else
      goto checkOpperation;
  } else
    scanner->setUk(pos, row, col);

  int type2;
  switch (scanner->next()) {
  case types::ID:
    scanner->getUk(pos, row, col);
    type2 = scanner->next();
    if (type2 == types::sLBKT) {
      expression();
      if (scanner->next() != types::sRBKT)
        printErr("Ожидалось ]");
      if (scanner->next() != types::sLBKT)
        printErr("Ожидалось [");
      expression();
      if (scanner->next() != types::sRBKT)
        printErr("Ожидалось ]");
    } else if (type2 == types::LBKT) {
      if (scanner->next() != types::RBKT)
        printErr("Ожидалось )");
    } else
      scanner->setUk(pos, row, col);

    break;
  case types::BOOL_CONST:
  case types::INT_CONST:
    break;
  default:
    printErr("Ожидался элемент");
    return;
    break;
  }

checkOpperation:
  scanner->getUk(pos, row, col);
  type = scanner->next();
  if (type == types::SUM || type == types::SUB || type == types::MUL || type == types::DIV || type == types::MULEQ || type == types::SUBEQ || type == types::SUMEQ || type == types::DIVEQ)
    element();
  else if (type == types::INC || type == types::DEC)
    return;
  else
    scanner->setUk(pos, row, col);
}

void Parser::printErr(const char *err)
{
  int pos, coll, row;
  this->scanner->getUk(pos, row, coll);
  printf("[Parser] %s:%d:%d Error: %s\n", this->scanner->getFilename(), row + 1, coll, err);
  errors++;

  throw "";
  //return types::ERROR;
}
