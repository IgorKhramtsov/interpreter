#include "treeNode.h"

std::shared_ptr<treeNode> treeNode::addId(const std::string_view &id_, const DataType dtype_, const IdType itype_, int fdimsize_, int sdimsize_)
{
  if (this->m_LeftNode != nullptr) {
    throw "Cant add treeNode to right. leftNode already exist";
  }

  this->m_LeftNode = std::make_shared<treeNode>(this, id_, dtype_, itype_, fdimsize_, sdimsize_);


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
