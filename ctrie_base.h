#ifndef _CTRIE_BASE_H
#define _CTRIE_BASE_H
 
namespace ctrie {

template<typename T, template<u_char> class Next, class Alloc>
class _BaseNode {
public:
  typedef _BaseNode<T,Next,Alloc> NodeT;
  typedef _Leaf<T,Next,Alloc> LeafT;

  class FindRtn {
  public:
    NodeT* node;
    size_t index;
    int cmpValue;

    FindRtn(NodeT* _node, size_t _index, int _cmpValue)
      : node(_node), index(_index), cmpValue(_cmpValue) {}
  };

  class InsertRtn {
  public:
    NodeT* node;
    size_t index;
    bool succeeded;

    InsertRtn(NodeT* _node, size_t _index, bool _succeeded)
      : node(_node), index(_index), succeeded(_succeeded) {}
  };

private:
  char* mStr;
  size_t mStrLen;

public:
  static size_t valueIndex()                  {return static_cast<size_t>(-1);}
  static size_t endIndex()                    {return static_cast<size_t>(-2);}

  virtual NodeT* clone() const = 0;
  virtual NodeT* moveAddValue(const T&) { assert(false); }
  virtual NodeT* moveRemoveValue()                            {assert(false);}
  virtual void destroy() = 0;
  _BaseNode& operator=(const _BaseNode&) = delete;

  void setStr(const char* str, size_t len);
  size_t strLen() const                                       {return mStrLen;}
  char* str()                                                 {return mStr;}
  const char* str() const                                     {return mStr;}

  virtual bool hasValue() const                               {return false;}
  virtual T& value()                                          {assert(false);}
  virtual const T& value() const                              {assert(false);}
  virtual T&& valueToMove()                                   {assert(false);}

  static InsertRtn insert(NodeT** node, const char* searchKey,
      size_t searchKeyLen, size_t pos, const T& value);
  static NodeT* createNode(NodeT* parent, const char* str, size_t strLen,
      char parentIndex);
  static NodeT* createNode(NodeT* parent, const char* str, size_t strLen,
      char parentIndex, const T& value);
  static NodeT* createNode(NodeT* parent, const char* str, size_t strLen,
      char parentIndex, T&& value);

  virtual bool empty() const                                  {assert(false);}
  virtual size_t size() const = 0;
  virtual size_t treeSize() const = 0;
  virtual NodeT* parent() const                               {assert(false);}
  virtual void setParent(NodeT*)                              {assert(false);}
  virtual char parentIndex() const                            {assert(false);}
  virtual void setParentIndex(char)                           {assert(false);}
  FindRtn find(const char* searchKeyData, size_t searchKeyLen);
  virtual char key(size_t)                                    {assert(false);}

  virtual NodeT** getEntryPtr(size_t)                         {assert(false);}
  virtual NodeT* getEntry(size_t) const                       {assert(false);}
  virtual size_t insertEntry(NodeT*, size_t, char, NodeT** p=nullptr)
                         {assert(false); assert(p); /* eliminate a warning */}
  virtual size_t eraseEntry(size_t, NodeT**)                  {assert(false);}
  virtual std::pair<size_t, bool> findEntry(char) const       {assert(false);}
  virtual size_t firstEntry() const                           {assert(false);}
  virtual size_t lastEntry() const                            {assert(false);}
  virtual size_t nextEntry(size_t) const                      {assert(false);}
  virtual size_t prevEntry(size_t) const                      {assert(false);}

protected:
  _BaseNode(const char* str, size_t len);
  _BaseNode(const NodeT& src);
  virtual ~_BaseNode();

private:
  static size_t matchLength(const char* s1, const char* s2, size_t len);
};

template<typename T, template<u_char> class Next, class Alloc>
inline
_BaseNode<T,Next,Alloc>::_BaseNode(const char* str, size_t len)
  : mStr(nullptr),
    mStrLen(0)
{
  setStr(str, len);
}

template<typename T, template<u_char> class Next, class Alloc>
inline
_BaseNode<T,Next,Alloc>::_BaseNode(const NodeT& src)
  : mStr(nullptr),
    mStrLen(0)
{
  setStr(src.mStr, src.mStrLen);
}

template<typename T, template<u_char> class Next, class Alloc>
inline
_BaseNode<T,Next,Alloc>::~_BaseNode()
{
  delete [] mStr;
  mStr = nullptr;
  mStrLen = 0;
}

template<typename T, template<u_char> class Next, class Alloc>
void
_BaseNode<T,Next,Alloc>::setStr(const char* str, size_t len)
{
  // FIXME I should be using Alloc, not new/delete.  But this means I need to
  // keep around the size, which is really a pain.
  if (len == 0 || mStrLen < len) {
    delete [] mStr;
    mStr = nullptr;
    mStrLen = 0;
  }
  if (len > 0) {
    if (mStr == nullptr) {
      mStr = new char[len];
    }
    memmove(mStr, str, len * sizeof(char));
    mStrLen = len;
  }
}

template<typename T, template<u_char> class Next, class Alloc>
typename _BaseNode<T,Next,Alloc>::InsertRtn
_BaseNode<T,Next,Alloc>::insert(NodeT** node, const char* searchKey,
    size_t searchKeyLen, size_t pos, const T& value)
{
  NodeT* origNode = *node;
  size_t nodeStrLen = origNode->strLen();
  if (nodeStrLen) {
    char *nodeStr = origNode->str();
    size_t matchLen = matchLength(
        nodeStr, searchKey + pos, std::min(nodeStrLen, searchKeyLen - pos));
    if (matchLen != nodeStrLen) {
      // The search key only matched part of this node's string, so we need to
      // break up the string into two parts.  We create a new node that is in
      // between this node and this node's parent.  The new node gets the
      // beginning of the string and this node gets the latter part of the
      // string.
      char indexCh = nodeStr[matchLen];
      bool insertValue = (pos + matchLen == searchKeyLen);
      if (insertValue) {
        *node = NodeT::createNode(origNode->parent(), nodeStr, matchLen,
            origNode->parentIndex(), value);
      } else {
        *node = NodeT::createNode(
            origNode->parent(), nodeStr, matchLen, origNode->parentIndex());
      }
      origNode->setStr(nodeStr + matchLen + 1, nodeStrLen - matchLen - 1);
      (*node)->insertEntry(origNode, 0, indexCh);
      if (insertValue) {
        return InsertRtn(*node, valueIndex(), true);
      }
      origNode = *node;
    }
    pos += matchLen;
  }

  if (searchKeyLen == pos) {
    // We have matched the entire key, so put the value in this node if
    // a value doesn't already exist
    if (origNode->hasValue()) {
      return InsertRtn(origNode, valueIndex(), false);
    }

    *node = origNode->moveAddValue(value);
    origNode->destroy();
    return InsertRtn(*node, valueIndex(), true);
  }
  
  // Find the next character of the search key in the node table.
  char searchCh = searchKey[pos++];
  std::pair<size_t, bool> findRtn = origNode->findEntry(searchCh);
  size_t index = findRtn.first;
  if (!findRtn.second) {
    // The next character of the search key is not in the array, so
    // insert a new leaf node into the vector.
    LeafT *leaf = LeafT::create(searchKey + pos, searchKeyLen - pos, value);
    index = origNode->insertEntry(leaf, index, searchCh, node);
    return InsertRtn(*node, index, true);
  }

  // The next character of the search key is in the table.  If the entry is
  // not a leaf node, traverse down to that node for further searching.
  NodeT** entry = origNode->getEntryPtr(index);
  LeafT* leaf = dynamic_cast<LeafT*>(*entry);
  if (leaf == nullptr) {
    return insert(entry, searchKey, searchKeyLen, pos, value);
  }

  // We have hit a leaf node.  Compare the leaf node and search strings.
  size_t leafStrLen = leaf->strLen();
  const char* leafStr = leaf->str();
  size_t matchLen = matchLength(
      leafStr, searchKey + pos, std::min(leafStrLen, searchKeyLen - pos));
  if (matchLen == leafStrLen && pos + matchLen == searchKeyLen) {
    // All of the search key matches all of the leaf node string,
    // so we are trying to insert a duplicate entry.
    return InsertRtn(origNode, index, false);
  }

  if (pos + matchLen == searchKeyLen) {
    // The insertion value goes into a new node, and the existing leaf node
    // becomes a child of the new node.
    *entry = NodeT::createNode(origNode, leafStr, matchLen, searchCh, value);
    (*entry)->insertEntry(leaf, 0, leafStr[matchLen]);
    leaf->setStr(leafStr + matchLen + 1, leafStrLen - matchLen - 1);
    return InsertRtn(*entry, valueIndex(), true);
  }

  NodeT *newLeaf = LeafT::create(
      searchKey + pos + matchLen + 1, searchKeyLen - pos - matchLen - 1, value);
  if (matchLen == leafStrLen) {
    // Move the leaf node to a non-leaf node, and the insertion value goes into
    // a new leaf node.
    *entry = NodeT::createNode(
        origNode, leafStr, leafStrLen, searchCh, leaf->valueToMove());
    index = (*entry)->insertEntry(newLeaf, 0, searchKey[pos + matchLen]);
    leaf->destroy();
  } else {
    // The new value and the existing leaf node are both children of a new
    // node.
    *entry = NodeT::createNode(origNode, leafStr, matchLen, searchCh);
    if (searchKey[pos + matchLen] < leafStr[matchLen]) {
      index = (*entry)->insertEntry(newLeaf, 0, searchKey[pos + matchLen]);
      (*entry)->insertEntry(leaf, 1, leafStr[matchLen]);
    } else {
      (*entry)->insertEntry(leaf, 0, leafStr[matchLen]);
      index = (*entry)->insertEntry(newLeaf, 1, searchKey[pos + matchLen]);
    }
    leaf->setStr(leafStr + matchLen + 1, leafStrLen - matchLen - 1);
  }
  return InsertRtn(*entry, index, true);
}

// output:
//    return value - The node that at least partially matches the given key
//        and an integer that tells how the search key is ordered with respect
//        to this node:
//          < 0 - The node key is lexically before the search key.
//          0   - The node key is equal to the search key.
//          1   - The prefix of the node key matches the search key.  For a
//                partial match, the nodes would be considered a match;
//                otherwise, the node key is lexically after the search key.
//          > 1 - The node key is after the search key.
//
template<typename T, template<u_char> class Next, class Alloc>
typename _BaseNode<T,Next,Alloc>::FindRtn
_BaseNode<T,Next,Alloc>::find(const char* searchKeyData, size_t searchKeyLen)
{
  // Compare to the string in this node if there is one.
  size_t nodeStrLen = strLen();
  if (nodeStrLen) {
    const char* nodeStr = str();

    // Compare the keys
    if (searchKeyLen >= nodeStrLen) {
      // We need the search key to match the entire node key, or no
      // match.  If it doesn't match, we are done searching.
      int cmp = memcmp(nodeStr, searchKeyData, nodeStrLen);
      if (cmp > 0) {
        return FindRtn(this, valueIndex(), 2);
      } else if (cmp < 0) {
        return FindRtn(this, endIndex(), -1);
      }
    } else {
      int cmp = memcmp(nodeStr, searchKeyData, searchKeyLen);
      // If cmp == 0, the node key is longer than the search key, but the
      // search key matches a prefix of the node key, so we want to
      // return 1.  Otherwise, return <0 or >1.
      if (cmp >= 0)
        return FindRtn(this, valueIndex(), cmp+1);
      else
        return FindRtn(this, endIndex(), cmp);
    }
    searchKeyData += nodeStrLen;
    searchKeyLen -= nodeStrLen;
  }
  if (searchKeyLen == 0) {
    // The search key matches this node, so return this node.  If there
    // is a value, then this is an exact match; otherwise, it is a partial
    // match.
    return FindRtn(this, valueIndex(), !hasValue());
  }

  // Now lookup the next character of the search string in the table
  std::pair<size_t, bool> findResult = findEntry(*searchKeyData);
  size_t index = findResult.first;
  if (!findResult.second) {
    // For an uncompressed table, advance to the next non-zero entry.
    if (index != endIndex() && getEntry(index) == nullptr)
      index = nextEntry(index);
    if (index == endIndex()) {
      // There are no entries that are >= *searchKeyData.  The keys
      // match in the parent, so the search key is after the parent node.
      return FindRtn(this, endIndex(), -1);
    } else {
      return FindRtn(this, index, 2);
    }
  }

  ++searchKeyData;
  --searchKeyLen;
  NodeT* entry = getEntry(index);
  if (dynamic_cast<LeafT*>(entry) == nullptr) {
    // A non-leaf node, so continue searching down the tree.
    return entry->find(searchKeyData, searchKeyLen);
  }

  // Figure out how much of the two strings match
  size_t minLen = std::min(searchKeyLen, entry->strLen());
  int cmp = memcmp(entry->str(), searchKeyData, minLen);
  if (cmp == 0) {
    cmp = static_cast<int>(entry->strLen()) - static_cast<int>(searchKeyLen);
    if (cmp > 0) {
      cmp = 1;
    }
  } else if (cmp > 0) {
    ++cmp;
  }
  return FindRtn(this, index, cmp);
}

template<typename T, template<u_char> class Next, class Alloc>
inline _BaseNode<T,Next,Alloc>*
_BaseNode<T,Next,Alloc>::createNode(NodeT* parent, const char* str,
    size_t strLen, char parentIndex)
{
  return _CmprNode<T,Next<std::numeric_limits<u_char>::max()>::up,Next,Alloc>::
      create(parent, str, strLen, parentIndex);
}

template<typename T, template<u_char> class Next, class Alloc>
inline _BaseNode<T,Next,Alloc>*
_BaseNode<T,Next,Alloc>::createNode(NodeT* parent, const char* str,
    size_t strLen, char parentIndex, const T& value)
{
  return _CmprValueNode<T,Next<std::numeric_limits<u_char>::max()>::up,Next,
         Alloc>::
      create(parent, str, strLen, parentIndex, value);
}

template<typename T, template<u_char> class Next, class Alloc>
inline _BaseNode<T,Next,Alloc>*
_BaseNode<T,Next,Alloc>::createNode(NodeT* parent, const char* str,
    size_t strLen, char parentIndex, T&& valueToMove)
{
  return _CmprValueNode<T,Next<std::numeric_limits<u_char>::max()>::up,Next,
         Alloc>::
      create(parent, str, strLen, parentIndex, valueToMove);
}

template<typename T, template<u_char> class Next, class Alloc>
inline size_t
_BaseNode<T,Next,Alloc>::matchLength(const char* s1, const char* s2, size_t len)
{
  for (size_t count = 0; count < len; ++count) {
    if (*s1++ != *s2++) {
      return count;
    }
  }
  return len;
}

} // end namespace ctrie
#endif
