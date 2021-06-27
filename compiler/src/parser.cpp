//parser.cpp
#include "parser.h"
#include "defs.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <charconv>
#include <variant>
#include "treeNode.h"

static bool isVarType(int type) { return (type == types::INT || type == types::BOOL); }

Parser::Parser(Scanner *sc)
{
  this->scanner = sc;
  this->m_Analyzer = new Analyzer(this->scanner);
  std::cout << "Running parser...\n";
  std::cout << "Running analyzer...\n";

  this->setInterp(true);
}

void Parser::s()
{
  LOG("S");
  auto type = scanner->next();

  if (isVarType(type)) {
    funcOrVar(type);
    s();
  } else if (type == types::END) {
    return;
  } else {
    printErr((std::string("Unrecognized token: ") + std::string(this->scanner->getToken().data())).c_str());
  }
}

void Parser::startInterp()
{
  setInterp(true);
  setFirstPass(false);
  auto ret = callFunc("main");
  std::cout << "Program exit with code " << std::get<int>(ret) << '\n';
}

void Parser::show() {
  std::cout << '\n' << "========Semantic tree========" << '\n';
  this->m_Analyzer->show();
}

data_variant Parser::callFunc(const std::string_view &name, std::vector<data_variant> args)
{
  auto m1 = m_Analyzer->findById(resolve_func(types::INT, name, args), true);
  auto m2 = m_Analyzer->findById(resolve_func(types::BOOL, name, args), true);
  auto m3 = m_Analyzer->findById(resolve_func(types::VOID, name, args), true);
  auto node = m1 ? m1 : m2 ? m2 : m3;

  if (node == nullptr) {
    this->m_Analyzer->printErr(std::string("Could not find method with name ").append(name).c_str());
  }
  return callFunc(node, args);
}

data_variant Parser::callFunc(std::shared_ptr<treeNode> node, std::vector<data_variant> args)
{
  if (this->FlagInterp) {

    this->callStack.push(this->scanner->getUk());
    auto tmp = this->m_Analyzer->getCurr();
    this->m_Analyzer->setCurr(node);
    this->scanner->setUk(*node->getUk());
    this->codeBlock(node, args);
    this->m_Analyzer->setCurr(tmp);
    this->scanner->setUk(this->callStack.top());
    this->callStack.pop();

    return node->getVal();
  } else {
    return -1;
  }
}

void Parser::funcOrVar(int type_)
{
  LOG("funcOrVar");

  int pos, col, row;// NOLINT
  scanner->getUk(pos, row, col);
  if (scanner->next() != types::ID) printErr("Identifier expected");
  const auto id = scanner->getToken();

  switch (scanner->next()) {
  case types::LBKT:
    scanner->setUk(pos, row, col);
    function(type_, id);
    break;
  case types::ASSIGN:
  case types::sLBKT:
  case types::SEMI:
  case types::COMA:
    scanner->setUk(pos, row, col);
    variables(type_);
    break;
  default:
    printErr("Variable or function expected");
  }
}

void Parser::function(int type_, const std::string_view &id_)
{
  LOG("function");

  if (scanner->next() != types::ID) printErr("Identifier expected");
  if (scanner->next() != types::LBKT) printErr("Expect (");
  auto arg_map = params();
  if (scanner->next() != types::RBKT) printErr("Expect )");

  m_Analyzer->addFunction((types)type_, resolve_func(type_, id_, arg_map), this->scanner->getUk(), arg_map);

  auto tmp = this->FlagInterp;
  setInterp(false);
  codeBlock(this->m_Analyzer->getCurr());
  setInterp(tmp);
}

std::map<std::string, int> Parser::params()
{
  auto map = std::map<std::string, int>();

  int next_type;
  do {
    auto saved_uk = this->scanner->getUk();
    next_type = this->scanner->next();
    if (next_type == types::INT || next_type == types::BOOL) {
      if (this->scanner->next() != types::ID) printErr("Argument name expected");
      if (map.find(this->scanner->getToken().data()) != map.end()) printErr("Argument with the same name already exist");
      map.insert( std::pair(std::string(this->scanner->getToken()), next_type) );
      if (this->scanner->peek() == types::COMA) {
        this->scanner->next();
        continue;
      } else {
        break;
      }

    } else if (next_type == types::RBKT) {
      this->scanner->setUk(saved_uk);
      break;
    } else {
      this->printErr("Unexpected symbol");
    }
  } while (true);

  return map;
}

std::vector<data_variant> Parser::arguments()
{
  auto list = std::vector<data_variant>();

  int next_type;
  do {
    auto saved_uk = this->scanner->getUk();
    next_type = this->scanner->next();
    if (next_type != types::RBKT) {
      this->scanner->setUk(saved_uk);
      auto exp_dat = expression();
      list.push_back(exp_dat.payload);

      if (this->scanner->peek() == types::COMA) {
        this->scanner->next();
        continue;
      } else {
        break;
      }

    } else if (next_type == types::RBKT) {
      this->scanner->setUk(saved_uk);
      break;
    } else {
      this->printErr("Unexpected symbol");
    }
  } while (true);

  return list;
}

void Parser::variables(int type_)
{
  LOG("variables");

  if (scanner->next() != types::ID) printErr("Identifier expected");

  const auto cachedToken = scanner->getToken();

  switch (scanner->next()) {
  case types::sLBKT: {
    if (scanner->next() != types::INT_CONST) printErr("Numeric const expected");
    const int size_fdim = std::stoi(scanner->getToken().data());
    if (scanner->next() != types::sRBKT) printErr("Expected ]");
    if (scanner->next() != types::sLBKT) printErr("Expected [");
    if (scanner->next() != types::INT_CONST) printErr("Numeric const expected");
    const int size_sdim = std::stoi(scanner->getToken().data());
    if (scanner->next() != types::sRBKT) printErr("Expected ]");

    m_Analyzer->addArr((types)type_, cachedToken, size_fdim, size_sdim);

    switch (scanner->next()) {
    case types::COMA:
      variables(type_);
      break;
    case types::SEMI:
    default:
      break;
    }
    break;
  }
  case types::ASSIGN: {

    auto exp_dat = expression();
    if (type_ != exp_dat.type) this->m_Analyzer->printErr("Variable is not subtype of expression result");
    this->m_Analyzer->addVar((types)type_, cachedToken, exp_dat.payload);

    auto type = scanner->next();
    if (type == types::COMA) {
      variables(type_);
    } else if (type != types::SEMI) {
      printErr("Expected ;");
    }
    break;
  }
  case types::SEMI:
    this->m_Analyzer->addVar((types)type_, cachedToken);
    break;
  case types::COMA:
    this->m_Analyzer->addVar((types)type_, cachedToken);
    variables(type_);
    break;
  default:
    this->printErr("Invalid token");
  }
}


void Parser::codeBlock(std::shared_ptr<treeNode> node_, std::vector<data_variant> args_)
{
  LOG("codeBlock");

  if (scanner->next() != types::fLBKT)
    printErr("Expected {");

  this->m_Analyzer->addScope();

  if (node_ != nullptr) {
    if (this->FlagInterp)
      this->m_Analyzer->setCurr(node_->propogateArgs(this->m_Analyzer->getCurr(), args_));
    else
      this->m_Analyzer->setCurr(node_->propogateArgs(this->m_Analyzer->getCurr()));
  }

  int pos, col, row;
  scanner->getUk(pos, row, col);
  auto type = scanner->next();

  while (type == types::INT || type == types::BOOL || type == types::IF || type == types::ID || type == types::fLBKT || type == types::RETURN) {
    if (type == types::INT || type == types::BOOL) {
      variables(type);
    } else if (type == types::IF) {
      if (scanner->next() != types::LBKT) printErr("Expected (");
      auto exp_dat = expression();
      if (exp_dat.type != types::BOOL) this->m_Analyzer->printErr("Cant assign to bool");
      if (scanner->next() != types::RBKT) printErr("Expected )");

      auto locFI = this->FlagInterp && std::get<bool>(exp_dat.payload);
      auto tmp = this->FlagInterp;
      setInterp(locFI);
      codeBlock();
      setInterp(tmp);
    } else if (type == types::ID) {
      const auto cachedId = this->scanner->getToken();
      // cachedId must be a node, so, in manipulating we can use it if it var, or arr

      auto typeOp = scanner->next();
      if (typeOp == types::sLBKT) {
        auto first_expr = expression();
        if (first_expr.type != types::INT) this->m_Analyzer->printErr("Invalid expression type");
        if (scanner->next() != types::sRBKT) printErr("Expected ]");
        if (scanner->next() != types::sLBKT) printErr("Expected [");
        auto second_expr = expression();
        if (second_expr.type != types::INT) this->m_Analyzer->printErr("Invalid expression type");
        if (scanner->next() != types::sRBKT) printErr("Expected ]");
        auto arrType = this->m_Analyzer->getTypeOf(cachedId, IdType::tArr);
        typeOp = scanner->next();

        switch (typeOp) {
        case types::ASSIGN: {

          auto exp_dat = expression();
          if (FlagInterp) {
            this->m_Analyzer->setArrVal(cachedId,
              std::get<int>(first_expr.payload),
              std::get<int>(second_expr.payload),
              exp_dat.payload);
          } else {
            if (exp_dat.type != this->m_Analyzer->getTypeOf(cachedId, IdType::tVar)) this->m_Analyzer->printErr("Variable is not subtype of expression result");
          }

          if (scanner->next() != types::SEMI) printErr("Expected ;");
          break;
        }
        case types::SUMEQ:
        case types::SUBEQ:
        case types::DIVEQ:
        case types::MULEQ: {
          auto exp_dat = expression();
          if (exp_dat.type != types::INT) this->m_Analyzer->printErr("Invalid expression type");

          if (this->FlagInterp) {
            int curVal = std::get<int>(this->m_Analyzer->getArrVal(cachedId,
              std::get<int>(first_expr.payload),
              std::get<int>(second_expr.payload)));
            int expVal = std::get<int>(exp_dat.payload);
            if (typeOp == types::SUMEQ) curVal += expVal;
            if (typeOp == types::SUBEQ) curVal -= expVal;
            if (typeOp == types::DIVEQ) curVal /= expVal;
            if (typeOp == types::MULEQ) curVal *= expVal;
            this->m_Analyzer->setArrVal(cachedId,
              std::get<int>(first_expr.payload),
              std::get<int>(second_expr.payload),
              curVal);
          }

          if (scanner->next() != types::SEMI) printErr("Expected ;");
          break;
        }
        case types::INC:
        case types::DEC: {

          if (FlagInterp) {
            auto value = std::get<int>(this->m_Analyzer->getArrVal(cachedId,
              std::get<int>(first_expr.payload),
              std::get<int>(second_expr.payload)));
            if (typeOp == types::INC) this->m_Analyzer->setArrVal(cachedId,
              std::get<int>(first_expr.payload),
              std::get<int>(second_expr.payload),
              value + 1);
            if (typeOp == types::DEC) this->m_Analyzer->setArrVal(cachedId,
              std::get<int>(first_expr.payload),
              std::get<int>(second_expr.payload),
              value - 1);
          } else {
            if (this->m_Analyzer->getTypeOf(cachedId, IdType::tVar) != types::INT) this->m_Analyzer->printErr("Invalid expression type");
          }

          if (scanner->next() != types::SEMI) printErr("Expected ;");
          break;
        }
        default:
          printErr("Expected assignment");
          break;
        }

      } else {
        switch (typeOp) {
        case types::LBKT: {
          auto args = arguments();
          if (scanner->next() != types::RBKT) printErr("Expected )");
          callFunc(cachedId, args);
          if (scanner->next() != types::SEMI) printErr("Expected ;");
          break;
        }
        case types::ASSIGN: {

          auto exp_dat = expression();
          if (FlagInterp) {
            this->m_Analyzer->setVarVal(cachedId, exp_dat.payload);
          } else {
            if (exp_dat.type != this->m_Analyzer->getTypeOf(cachedId, IdType::tVar)) this->m_Analyzer->printErr("Variable is not subtype of expression result");
          }

          if (scanner->next() != types::SEMI) printErr("Expected ;");
          break;
        }
        case types::SUMEQ:
        case types::SUBEQ:
        case types::DIVEQ:
        case types::MULEQ: {
          auto exp_dat = expression();
          if (exp_dat.type != types::INT) this->m_Analyzer->printErr("Invalid expression type");

          if (this->FlagInterp) {
            int curVal = std::get<int>(this->m_Analyzer->getVarVal(cachedId));
            int expVal = std::get<int>(exp_dat.payload);
            if (typeOp == types::SUMEQ) curVal += expVal;
            if (typeOp == types::SUBEQ) curVal -= expVal;
            if (typeOp == types::DIVEQ) curVal /= expVal;
            if (typeOp == types::MULEQ) curVal *= expVal;
            this->m_Analyzer->setVarVal(cachedId, curVal);
          }

          if (scanner->next() != types::SEMI) printErr("Expected ;");
          break;
        }
        case types::INC:
        case types::DEC: {

          if (FlagInterp) {
            auto value = std::get<int>(this->m_Analyzer->getVarVal(cachedId));
            if (typeOp == types::INC) this->m_Analyzer->setVarVal(cachedId, value + 1);
            if (typeOp == types::DEC) this->m_Analyzer->setVarVal(cachedId, value - 1);
          } else {
            if (this->m_Analyzer->getTypeOf(cachedId, IdType::tVar) != types::INT) this->m_Analyzer->printErr("Invalid expression type");
          }

          if (scanner->next() != types::SEMI) printErr("Expected ;");
          break;
        }
        default:
          printErr("Call or assignment was expected");
          break;
        }
      }
    } else if (type == types::fLBKT) {
      scanner->setUk(pos, row, col);
      codeBlock();
    } else if (type == types::RETURN) {
      auto exp_dat = expression();
      if (exp_dat.type != this->m_Analyzer->getTypeOfFunc()) this->m_Analyzer->printErr("Expression type is not subtype of fuction result");
      if (FlagInterp) this->m_Analyzer->setFuncRet(exp_dat.payload);

      if (scanner->next() != types::SEMI) printErr("Expected ;");

      if (this->FlagInterp) return;
    }

    scanner->getUk(pos, row, col);
    type = scanner->next();
  }
  scanner->setUk(pos, row, col);

  if (scanner->next() != types::fRBKT) printErr("Expected }");
  this->m_Analyzer->exitScope();
}

Data Parser::expression()
{
  LOG("expression");
  int pos, col, row;

  auto left_el = element();
  auto resType = left_el.type;
  data_variant resVal;

  scanner->getUk(pos, row, col);
  auto type = scanner->next();
  switch (type) {
  case types::MORE:
  case types::LESS:
  case types::MEQ:
  case types::LEQ:
  case types::EQ: {

    auto right_el = expression();
    if (left_el.type != right_el.type) this->m_Analyzer->printErr("Expression have different type");
    resType = BOOL;

    if (FlagInterp) {
      switch (type) {
      case types::MORE:
        resVal = left_el.payload > right_el.payload;
        break;
      case types::LESS:
        resVal = left_el.payload < right_el.payload;
        break;
      case types::MEQ:
        resVal = left_el.payload >= right_el.payload;
        break;
      case types::LEQ:
        resVal = left_el.payload <= right_el.payload;
        break;
      case types::EQ:
        resVal = left_el.payload == right_el.payload;
        break;
      }
    }

    break;
  }

  default:
    scanner->setUk(pos, row, col);
    resVal = left_el.payload;
    break;
  }

  return Data(resType, resVal);
}

Data Parser::element()
{
  LOG("element");
  int resType;
  data_variant resVal;
  std::string left_name;

  // a = (a) | a
  int pos, col, row;
  scanner->getUk(pos, row, col);
  if (scanner->next() == types::LBKT) {
    auto el = element();
    resType = el.type;
    resVal = el.payload;
    if (scanner->next() != types::RBKT) printErr("Expected )");
    goto checkOpperation;
  } else {
    scanner->setUk(pos, row, col);
  }
  // -------

  switch (scanner->next()) {
  case types::ID: {
    const auto cached_id = scanner->getToken();

    scanner->getUk(pos, row, col);
    auto type = scanner->next();
    if (type == types::sLBKT) {
      auto first_expr = expression();
      if (first_expr.type != types::INT) this->m_Analyzer->printErr("Invalid expression type");
      if (scanner->next() != types::sRBKT) printErr("Expected ]");
      if (scanner->next() != types::sLBKT) printErr("Expected [");
      auto second_expr = expression();
      if (second_expr.type != types::INT) this->m_Analyzer->printErr("Invalid expression type");
      if (scanner->next() != types::sRBKT) printErr("Expected ]");
      resType = this->m_Analyzer->getTypeOf(cached_id, IdType::tArr);
      resVal = this->m_Analyzer->getArrVal(cached_id, std::get<int>(first_expr.payload), std::get<int>(second_expr.payload));
    } else if (type == types::LBKT) {
      auto args = arguments();
      if (scanner->next() != types::RBKT) printErr("Expected )");
      resType = this->m_Analyzer->getTypeOf(cached_id, IdType::tFunc, args);
      resVal = callFunc(cached_id, args);
    } else {
      resType = this->m_Analyzer->getTypeOf(cached_id, IdType::tVar);
      resVal = this->m_Analyzer->getVarVal(cached_id);
      left_name = cached_id;
      scanner->setUk(pos, row, col);
    }

    break;
  }
  case types::BOOL_CONST:
    resType = BOOL;
    resVal = strcmp(this->scanner->getToken().data(), "true") == 1 ? true : false;
    break;
  case types::INT_CONST:
    resType = INT;
    resVal = std::stoi(this->scanner->getToken().data());
    break;
  default:
    printErr("Element expected");
  }

checkOpperation:
  scanner->getUk(pos, row, col);
  auto type = scanner->next();
  if (type == types::SUM || type == types::SUB || type == types::MUL || type == types::DIV || type == types::MULEQ || type == types::SUBEQ || type == types::SUMEQ || type == types::DIVEQ) {
    auto right_el = element();
    auto left_el = Data(resType, resVal);
    if (resType != types::INT || right_el.type != types::INT) this->m_Analyzer->printErr("Invalid element type (only integer allowed)");


    if (this->FlagInterp) {
      auto leftVal = std::get<int>(left_el.payload);
      auto rightVal = std::get<int>(right_el.payload);
      switch (type) {
      case types::SUM:
        resVal = leftVal + rightVal;
        break;
      case types::SUB:
        resVal = leftVal - rightVal;
        break;
      case types::MUL:
        resVal = leftVal * rightVal;
        break;
      case types::DIV:
        resVal = leftVal / rightVal;
        break;
      }
      if (!left_name.empty() && (type == types::SUMEQ || type == types::SUBEQ || type == types::MULEQ || type == types::DIVEQ)) {
        switch (type) {
        case types::SUMEQ:
          leftVal = leftVal + rightVal;
          break;
        case types::SUBEQ:
          leftVal = leftVal - rightVal;
          break;
        case types::MULEQ:
          leftVal = leftVal * rightVal;
          break;
        case types::DIVEQ:
          leftVal = leftVal / rightVal;
          break;
        }
        this->m_Analyzer->setVarVal(left_name, leftVal);
        resVal = leftVal;
      }
    }

  } else if (type == types::INC || type == types::DEC) {
    if (!left_name.empty()) {
      auto resValInt = std::get<int>(resVal);
      if (type == types::INC) resValInt++;
      if (type == types::DEC) resValInt--;

      if (this->FlagInterp) this->m_Analyzer->setVarVal(left_name, resValInt);
      resVal = data_variant(resValInt);
    } else {
      this->m_Analyzer->printErr("Operation can be applied only to variables");
    }

  } else {
    scanner->setUk(pos, row, col);
  }


  return Data(resType, resVal);
}

void Parser::printErr(const char *err)
{

  int pos, coll, row;
  this->scanner->getUk(pos, row, coll);
  std::cout << "[Parser] " << this->scanner->getFilename() << ":" << row + 1 << ":" << coll << " Error " << err << '\n';
  errors++;

  throw "";
  //return types::ERROR;
}
