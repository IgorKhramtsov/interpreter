//treeNode.cpp
#include "treeNode.h"

std::shared_ptr<treeNode> treeNode::addArr(const std::string_view &id_, const DataType dtype_, int fdimsize_, int sdimsize_)
{
  if (this->m_LeftNode != nullptr) {
    throw "Cant add treeNode to left. leftNode already exist";
  }

  this->m_LeftNode = std::make_shared<treeNode>(this, id_, dtype_, IdType::tArr, fdimsize_, sdimsize_);
  if (dtype_ == DataType::tInt) {
    int **arr = new int *[fdimsize_];
    for (int i = 0; i < fdimsize_; i++)
      arr[i] = new int[sdimsize_];
    this->m_LeftNode->value = arr;
  } else {
    bool **arr = new bool *[fdimsize_];
    for (int i = 0; i < fdimsize_; i++)
      arr[i] = new bool[sdimsize_];
    this->m_LeftNode->value = arr;
  }


  return this->m_LeftNode;
}

std::shared_ptr<treeNode> treeNode::addId(const std::string_view &id_, const DataType dtype_, data_variant dat)
{
  if (this->m_LeftNode != nullptr) {
    throw "Cant add treeNode to left. leftNode already exist";
  }

  this->m_LeftNode = std::make_shared<treeNode>(this, id_, dtype_, IdType::tVar);
  this->m_LeftNode->value = dat;


  return this->m_LeftNode;
}

std::shared_ptr<treeNode> treeNode::addFunc(const std::string_view &id_, const DataType dtype_, Uk uk_)
{
  if (this->m_LeftNode != nullptr) {
    throw "Cant add treeNode to left. leftNode already exist";
  }

  this->m_LeftNode = std::make_shared<treeNode>(this, id_, dtype_, IdType::tFunc);
  this->m_LeftNode->scope_uk = std::make_unique<Uk>(uk_);


  return this->m_LeftNode;
}

std::shared_ptr<treeNode> treeNode::addScope()
{
  if (this->m_RigthNode != nullptr) {
    throw "Cant add treeNode to right. rightNode already exist";
  }

  this->m_RigthNode = std::make_shared<treeNode>(this);


  return this->m_RigthNode;
}

std::shared_ptr<treeNode> treeNode::exitScope()
{
  auto par = this->m_Parent;
  while (par->m_RigthNode == nullptr) {
    par = par->m_Parent;
  }
  return par;
}

std::shared_ptr<treeNode> treeNode::getParent()
{
  return this->m_Parent;
}

treeNode *treeNode::search(const std::string_view &search_id)
{
  if (this->m_Id == search_id)
    return this;
  else if (m_Parent != nullptr)
    return this->m_Parent->search(search_id);
  else
    return nullptr;
}

std::shared_ptr<treeNode> treeNode::searchDown(const std::string_view &search_id)
{
  if (this == nullptr || (!this->m_LeftNode && !this->m_RigthNode))
    return nullptr;

  if (this->m_LeftNode)
    if (this->m_LeftNode->m_Id == search_id)
      return this->m_LeftNode;
  if (this->m_RigthNode)
    if (this->m_RigthNode->m_Id == search_id)
      return this->m_RigthNode;

  auto l = this->m_LeftNode->searchDown(search_id);
  auto r = this->m_RigthNode->searchDown(search_id);
  return l ? l : r;
}

const std::shared_ptr<treeNode> treeNode::makeRoot() noexcept
{
  return std::make_shared<treeNode>(nullptr);
}