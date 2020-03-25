#pragma once
#include "defs.h"
#include <string>

enum struct DataType {
  none = 0,// for scope
  tInt,
  tBool
};

enum struct IdType {
  none = 0,// for scope
  tVar,
  tFunc,
  tArr
};

class treeNode
{
  std::string m_Id;
  IdType m_IdType;
  DataType m_DataType;


  treeNode *m_LeftNode;// vars/funcs
  treeNode *m_RigthNode;// scopes
  treeNode *m_Parent;

  treeNode(treeNode *_par, const char *id_ = "", const DataType dtype_ = DataType::none, const IdType itype_ = IdType::none)
    : m_Parent{ _par }, m_Id{ id_ }, m_IdType{ itype_ }, m_DataType{ dtype_ }
  {}

public:
  treeNode *addId(const char *, const DataType, const IdType);
  treeNode *addScope();
  treeNode *search(const char *);
};
