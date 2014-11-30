#ifndef _CTRIE_CMPR_H
#define _CTRIE_CMPR_H
 
namespace ctrie {

template<typename T, u_char Sz, template<u_char Sz> class Next, class Alloc>
class _CmprNode : public _BaseNode<T,Next,Alloc> {
public:
  typedef _BaseNode<T,Next,Alloc> NodeT;
  typedef _Leaf<T,Next,Alloc> LeafT;
  typedef _CmprNode<T,Sz,Next,Alloc> CmprNodeT;
  typedef _FullNode<T,Next,Alloc> FullNodeT;
  typedef _CmprValueNode<T,Sz,Next,Alloc> CmprValueNodeT;
  typedef _FullValueNode<T,Next,Alloc> FullValueNodeT;

private:
  NodeT* mParent;
  u_char mNumChildren;
  u_char mCharTable[Sz + 1];
  NodeT* mChildren[Sz];

public:
  static _CmprNode*
      create(NodeT* parent, const char* str, size_t strLen, char parentIndex);
  template<u_char SrcSz>
    static _CmprNode* move(_CmprNode<T,SrcSz,Next,Alloc>& src);
  static _CmprNode* move(CmprNodeT& x);
  static _CmprNode* move(FullNodeT& x);
  NodeT* moveAddValue(const T& valueToAdd) /*override*/;
  NodeT* clone() const /*override*/;
  void destroy() /*override*/;
  _CmprNode& operator=(const CmprNodeT&) = delete;
  _CmprNode& operator=(CmprNodeT&&) = delete;

  bool empty() const /*override*/                {return mNumChildren == 0;}
  size_t size() const /*override*/               {return mNumChildren;};
  size_t treeSize() const /*override*/;
  NodeT* parent() const /*override*/             {return mParent;}
  void setParent(NodeT* parent)                  {mParent = parent;}
  char parentIndex() const /*override*/          {return (char) mCharTable[Sz];}
  void setParentIndex(char i) /*override*/       {mCharTable[Sz] = (u_char) i;}
  char key(size_t i) /*override*/                {return (char) mCharTable[i];}

  NodeT** getEntryPtr(size_t i) /*override*/     {return mChildren + i;}
  NodeT* getEntry(size_t i) const /*override*/   {return mChildren[i];}
  std::pair<size_t, bool> findEntry(char key) const /*override*/;
  size_t insertEntry(NodeT* entry, size_t index, char key, NodeT** replacement)
      /*override*/;
  size_t eraseEntry(size_t index, NodeT** newNode) /*override*/;
  size_t firstEntry() const /*override*/;
  size_t lastEntry() const /*override*/          {return mNumChildren - 1;}
  size_t nextEntry(size_t index) const /*override*/;
  size_t prevEntry(size_t index) const /*override*/;

protected:
  _CmprNode(NodeT* parent, const char* str, size_t strLen, char parentIndex);
  _CmprNode(const CmprNodeT& x);
  _CmprNode(CmprNodeT&& x);
  template<u_char SrcSz>
    _CmprNode(_CmprNode<T,SrcSz,Next,Alloc>&& src);
  _CmprNode(FullNodeT&& x);
  ~_CmprNode();

private:
  static CmprNodeT* allocate();
  void init();
  template<u_char SrcSz>
    void moveChildren(_CmprNode<T,SrcSz,Next,Alloc>& x);

  friend class _CmprNode<T,2,Next,Alloc>;
  friend class _CmprNode<T,4,Next,Alloc>;
  friend class _CmprNode<T,8,Next,Alloc>;
  friend class _CmprNode<T,16,Next,Alloc>;
  friend class _CmprNode<T,32,Next,Alloc>;
  friend class _CmprNode<T,std::numeric_limits<u_char>::max(),Next,Alloc>;
  friend class _FullNode<T,Next,Alloc>;
};

template<typename T, u_char Sz, template<u_char Sz> class Next, class Alloc>
class _CmprValueNode : public _CmprNode<T,Sz,Next,Alloc> {
private:
  T mValue;

public:
  typedef _BaseNode<T,Next,Alloc> NodeT;
  typedef _CmprNode<T,Sz,Next,Alloc> CmprNodeT;
  typedef _CmprValueNode<T,Sz,Next,Alloc> ValueNodeT;

  static ValueNodeT* create(NodeT* parent, const char* str, size_t strLen,
      char parentIndex, const T& value);
  static ValueNodeT* create(NodeT* parent, const char* str, size_t strLen,
      char parentIndex, T&& value);
  static ValueNodeT* move(_CmprNode<T,Sz,Next,Alloc>& x, const T& value);
  template<u_char SrcSz>
    static ValueNodeT* move(_CmprValueNode<T,SrcSz,Next,Alloc>& x);
  static ValueNodeT* move(_FullValueNode<T,Next,Alloc>& x);

  NodeT* clone() const /*override*/;
  NodeT* moveRemoveValue() /*override*/        {return CmprNodeT::move(*this);}
  void destroy() /*override*/;
  _CmprValueNode(const ValueNodeT&&) = delete;
  ValueNodeT& operator=(const ValueNodeT&) = delete;
  ValueNodeT& operator=(const ValueNodeT&&) = delete;

  bool hasValue() const /*override*/           {return true;}
  T& value() /*override*/                      {return mValue;}
  const T& value() const /*override*/          {return mValue;}
  T&& valueToMove() /*override*/               {return std::move(mValue);}

protected:
  ~_CmprValueNode() {}

private:
  _CmprValueNode(NodeT* parent, const char* str, size_t strLen,
      char parentIndex, const T& value);
  _CmprValueNode(NodeT* parent, const char* str, size_t strLen,
      char parentIndex, T&& value);
  _CmprValueNode(CmprNodeT&& src, const T& value);
  _CmprValueNode(const ValueNodeT& src);
  _CmprValueNode(_FullValueNode<T,Next,Alloc>&& src);
  template<u_char SrcSz>
  _CmprValueNode(_CmprValueNode<T,SrcSz,Next,Alloc>&& src);

  static ValueNodeT* allocate();
};

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _CmprNode<T,Sz,Next,Alloc>::_CmprNode(
    NodeT* parent, const char* str, size_t strLen, char parentIndex)
  : _BaseNode<T,Next,Alloc>(str, strLen), mParent(parent), mNumChildren(0)
{
  init();
  setParentIndex(parentIndex);
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _CmprNode<T,Sz,Next,Alloc>::_CmprNode(const CmprNodeT& x)
  : _BaseNode<T,Next,Alloc>(x), mParent(x.mParent), mNumChildren(x.mNumChildren)
{
  init();
  std::copy(x.mCharTable, x.mCharTable + x.mNumChildren, mCharTable);
  for (u_char i = 0; i < mNumChildren; ++i) {
    mChildren[i] = x.mChildren[i]->clone();
    if (dynamic_cast<LeafT*>(mChildren[i]) == 0) {
      mChildren[i]->setParent(this);
    }
  }
  setParentIndex(x.parentIndex());
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
_CmprNode<T,Sz,Next,Alloc>::_CmprNode(_CmprNode<T,Sz,Next,Alloc>&& src)
  : _BaseNode<T,Next,Alloc>(src), mParent(0), mNumChildren(0)
{
  init();
  moveChildren(src);
  std::swap(mParent, src.mParent);
  std::swap(mNumChildren, src.mNumChildren);
  setParentIndex(src.parentIndex());
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
template<u_char SrcSz>
_CmprNode<T,Sz,Next,Alloc>::_CmprNode(_CmprNode<T,SrcSz,Next,Alloc>&& src)
  : _BaseNode<T,Next,Alloc>(src), mParent(0), mNumChildren(0)
{
  init();
  moveChildren(src);
  std::swap(mParent, src.mParent);
  std::swap(mNumChildren, src.mNumChildren);
  setParentIndex(src.parentIndex());
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
_CmprNode<T,Sz,Next,Alloc>::_CmprNode(FullNodeT&& src)
  : _BaseNode<T,Next,Alloc>(std::move(src)), mParent(0), mNumChildren(0)
{
  assert(src.mNumChildren <= Sz);
  init();

  size_t count = 0;
  for (size_t i = 0;
      i < FullNodeT::sMaxNumChildren && count <= src.mNumChildren; ++i) {
    NodeT** node = src.getEntryPtr(i);
    if (*node) {
      mCharTable[count] = static_cast<u_char>(i);
      mChildren[count] = *node;
      if (dynamic_cast<LeafT*>(*node) == 0) {
        (*node)->setParent(this);
      }
      *node = 0;
      ++count;
    }
  }
  assert(count == static_cast<size_t>(src.mNumChildren) + 1);
  std::swap(mParent, src.mParent);
  mNumChildren = static_cast<u_char>(src.mNumChildren + 1);
  setParentIndex(src.parentIndex());
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
_CmprNode<T,Sz,Next,Alloc>::~_CmprNode()
{
  for (u_char i = 0; i < mNumChildren; ++i) {
    mChildren[i]->destroy();
    mChildren[i] = nullptr;
  }
  mNumChildren = 0;
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _CmprNode<T,Sz,Next,Alloc>*
_CmprNode<T,Sz,Next,Alloc>::create(
    NodeT* parent, const char* str, size_t strLen, char parentIndex)
{
  return new(allocate()) CmprNodeT(parent, str, strLen, parentIndex);
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
template<u_char SrcSz>
inline _CmprNode<T,Sz,Next,Alloc>*
_CmprNode<T,Sz,Next,Alloc>::move(_CmprNode<T,SrcSz,Next,Alloc>& x)
{
  return new(allocate()) CmprNodeT(std::move(x));
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _CmprNode<T,Sz,Next,Alloc>*
_CmprNode<T,Sz,Next,Alloc>::move(CmprNodeT& x)
{
  return new(allocate()) CmprNodeT(std::move(x));
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _CmprNode<T,Sz,Next,Alloc>*
_CmprNode<T,Sz,Next,Alloc>::move(FullNodeT& x)
{
  return new(allocate()) CmprNodeT(std::move(x));
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _BaseNode<T,Next,Alloc>*
_CmprNode<T,Sz,Next,Alloc>::moveAddValue(const T& valueToAdd)
{
  return CmprValueNodeT::move(*this, valueToAdd);
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _BaseNode<T,Next,Alloc>*
_CmprNode<T,Sz,Next,Alloc>::clone() const
{
  return new(allocate()) CmprNodeT(*this);
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline void
_CmprNode<T,Sz,Next,Alloc>::destroy()
{
  typename Alloc::template rebind<CmprNodeT>::other alloc;
  this->~_CmprNode<T,Sz,Next,Alloc>();
  alloc.deallocate(this, 1);
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
size_t
_CmprNode<T,Sz,Next,Alloc>::treeSize() const
{
   size_t size = this->hasValue() ? 1 : 0;
   for (u_char i = 0; i < mNumChildren; ++i) {
      size += mChildren[i]->treeSize();
   }
   return size;
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _CmprNode<T,Sz,Next,Alloc>*
_CmprNode<T,Sz,Next,Alloc>::allocate()
{
  typename Alloc::template rebind<CmprNodeT>::other alloc;
  return alloc.allocate(1);
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
std::pair<size_t, bool>
_CmprNode<T,Sz,Next,Alloc>::findEntry(char key) const
{
  u_char ukey = static_cast<u_char>(key);
  if (mNumChildren <= 4) {
    // For small sizes, it is faster to just compare each one.
    if (mNumChildren == 0) {
      return std::make_pair(NodeT::endIndex(), false);
    }
    if (ukey < mCharTable[0]) {
      return std::make_pair(0, false);
    }
    for (size_t i = 1; i < mNumChildren; ++i) {
      if (ukey < mCharTable[i]) {
        if (ukey > mCharTable[i - 1]) {
          return std::make_pair(i, false);
        } else {
          return std::make_pair(i - 1, true);
        }
      }
    }
    if (ukey > mCharTable[mNumChildren - 1]) {
      return std::make_pair(NodeT::endIndex(), false);
    } else {
      return std::make_pair(mNumChildren - 1, true);
    }
  } else {
    const u_char *start = mCharTable;
    const u_char *end = mCharTable + mNumChildren - 1;
    const u_char *mid;
    char entry;

    while (true) {
      size_t diff = end - start;
      if (diff <= 1) {
        if (ukey > *start) {
          ++start;
          if (diff == 1 && ukey > *start)
            ++start;
        }
        size_t index = start - mCharTable;
        if (index == mNumChildren)
          return std::make_pair(NodeT::endIndex(), false);
        else
          return std::make_pair(index, *start == ukey);
      }
      mid = start + (diff >> 1);
      entry = *mid;
      if (ukey < entry) {
        end = mid - 1;
      } else if (ukey != entry) {
        start = mid + 1;
      } else {
        return std::make_pair(mid - mCharTable, true);
      }
    }
    return std::make_pair(NodeT::endIndex(), false);
  }
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
size_t
_CmprNode<T,Sz,Next,Alloc>::insertEntry(
    NodeT* entry, size_t index, char key, NodeT** replacement)
{
  if (dynamic_cast<LeafT*>(entry) == 0) {
    entry->setParent(this);
    entry->setParentIndex(key);
  }
  if (mNumChildren != Sz) {
    // Make room for the new entry in the compressed array
    if (index == NodeT::endIndex()) {
      index = mNumChildren;
    } else {
      memmove(mCharTable + index + 1, mCharTable + index,
          (mNumChildren - index) * sizeof(char));
      memmove(mChildren + index + 1, mChildren + index,
          (mNumChildren - index) * sizeof(void*));
    }
    mCharTable[index] = key;
    mChildren[index] = entry;
    ++mNumChildren;
    return index;
  }

  // This node has run out of room, so we need to replace this node with the
  // next size up.
  assert(replacement != nullptr);
  if (Next<Sz>::up == std::numeric_limits<u_char>::max()) {
    // Create an uncompressed node before inserting
    CmprValueNodeT *nodeWithValue = dynamic_cast<CmprValueNodeT*>(this);
    if (nodeWithValue) {
      *replacement = FullValueNodeT::move(*nodeWithValue);
    } else {
      *replacement = FullNodeT::move(*this);
    }
    this->destroy();
    return (*replacement)->insertEntry(
        entry, static_cast<size_t>(key), key, replacement);
  } else {
    CmprValueNodeT *nodeWithValue = dynamic_cast<CmprValueNodeT*>(this);
    if (this->hasValue()) {
      *replacement =
          _CmprValueNode<T,Next<Sz>::up,Next,Alloc>::move(*nodeWithValue);
    } else {
      *replacement = _CmprNode<T,Next<Sz>::up,Next,Alloc>::move(*this);
    }
    this->destroy();
    return (*replacement)->insertEntry(entry, index, key, replacement);
  }
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
size_t
_CmprNode<T,Sz,Next,Alloc>::eraseEntry(size_t index, NodeT** newNode)
{
  assert(mNumChildren != 0);
  *newNode = this;
  --mNumChildren;
  size_t nextIndex = index;
  if (index == mNumChildren) {
    nextIndex = NodeT::endIndex();
  } else {
    // Move everything over to fill in the erased entry.
    memmove(mCharTable + index, mCharTable + index + 1,
            (mNumChildren - index) * sizeof(char));
    memmove(mChildren + index, mChildren + index + 1,
            (mNumChildren - index) * sizeof(void*));
  }

  if (mNumChildren <= Next<Sz>::downThreshold) {
    CmprValueNodeT *nodeWithValue = dynamic_cast<CmprValueNodeT*>(this);
    if (nodeWithValue) {
      *newNode =
          _CmprValueNode<T,Next<Sz>::down,Next,Alloc>::move(*nodeWithValue);
    } else {
      *newNode = _CmprNode<T,Next<Sz>::down,Next,Alloc>::move(*this);
    }
  }
  return nextIndex;
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline size_t
_CmprNode<T,Sz,Next,Alloc>::firstEntry() const
{
  return empty() ? NodeT::endIndex() : 0;
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline size_t
_CmprNode<T,Sz,Next,Alloc>::nextEntry(size_t index) const
{
  if (index == NodeT::valueIndex()) {
    return firstEntry();
  } else {
    return ++index < mNumChildren ? index : NodeT::endIndex();
  }
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline size_t
_CmprNode<T,Sz,Next,Alloc>::prevEntry(size_t index) const
{
  if (index == NodeT::valueIndex() || index == 0) {
    return NodeT::valueIndex();
  } else if (index == NodeT::endIndex()) {
    return mNumChildren - 1;
  } else {
    return index - 1;
  }
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline void
_CmprNode<T,Sz,Next,Alloc>::init()
{
  std::fill(mCharTable, mCharTable + Sz, std::numeric_limits<u_char>::max());
  std::fill(mChildren, mChildren + Sz, nullptr);
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
template<u_char SrcSz>
inline void
_CmprNode<T,Sz,Next,Alloc>::moveChildren(_CmprNode<T,SrcSz,Next,Alloc>& src)
{
  std::swap_ranges(
      src.mCharTable, src.mCharTable + src.mNumChildren, mCharTable);
  std::swap_ranges(src.mChildren, src.mChildren + src.mNumChildren, mChildren);
  for (NodeT* child : mChildren) {
    if (child && dynamic_cast<LeafT*>(child) == 0) {
      child->setParent(this);
    }
  }
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline
_CmprValueNode<T,Sz,Next,Alloc>::_CmprValueNode(NodeT* parent, const char* str,
    size_t strLen, char parentIndex, const T& value)
  : _CmprNode<T,Sz,Next,Alloc>(parent, str, strLen, parentIndex), mValue(value)
{}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline
_CmprValueNode<T,Sz,Next,Alloc>::_CmprValueNode(NodeT* parent, const char* str,
    size_t strLen, char parentIndex, T&& value)
  : _CmprNode<T,Sz,Next,Alloc>(parent, str, strLen, parentIndex),
    mValue(std::move(value))
{}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline
_CmprValueNode<T,Sz,Next,Alloc>::_CmprValueNode(CmprNodeT&& src, const T& value)
  : _CmprNode<T,Sz,Next,Alloc>(std::move(src)),
    mValue(value)
{}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline
_CmprValueNode<T,Sz,Next,Alloc>::_CmprValueNode(const ValueNodeT& src)
  : _CmprNode<T,Sz,Next,Alloc>(src),
    mValue(src.value())
{}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline
_CmprValueNode<T,Sz,Next,Alloc>::_CmprValueNode(
    _FullValueNode<T,Next,Alloc>&& src)
  : _CmprNode<T,Sz,Next,Alloc>(std::move(src)),
    mValue(src.valueToMove())
{}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
template<u_char SrcSz>
inline
_CmprValueNode<T,Sz,Next,Alloc>::_CmprValueNode(
    _CmprValueNode<T,SrcSz,Next,Alloc>&& src)
  : _CmprNode<T,Sz,Next,Alloc>(std::move(src)),
    mValue(src.valueToMove())
{}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _CmprValueNode<T,Sz,Next,Alloc>*
_CmprValueNode<T,Sz,Next,Alloc>::create(NodeT* parent, const char* str,
    size_t strLen, char parentIndex, const T& value)
{
  return new(allocate()) ValueNodeT(parent, str, strLen, parentIndex, value);
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _CmprValueNode<T,Sz,Next,Alloc>*
_CmprValueNode<T,Sz,Next,Alloc>::create(NodeT* parent, const char* str,
    size_t strLen, char parentIndex, T&& value)
{
  return new(allocate()) ValueNodeT(parent, str, strLen, parentIndex, value);
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _CmprValueNode<T,Sz,Next,Alloc>*
_CmprValueNode<T,Sz,Next,Alloc>::move(
    _CmprNode<T,Sz,Next,Alloc>& x, const T& value)
{
  return new(allocate()) ValueNodeT(std::move(x), value);
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
template<u_char SrcSz>
inline _CmprValueNode<T,Sz,Next,Alloc>*
_CmprValueNode<T,Sz,Next,Alloc>::move(_CmprValueNode<T,SrcSz,Next,Alloc>& x)
{
  return new(allocate()) ValueNodeT(std::move(x));
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _CmprValueNode<T,Sz,Next,Alloc>*
_CmprValueNode<T,Sz,Next,Alloc>::move(_FullValueNode<T,Next,Alloc>& x)
{
  return new(allocate()) ValueNodeT(std::move(x));
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _BaseNode<T,Next,Alloc>*
_CmprValueNode<T,Sz,Next,Alloc>::clone() const
{
  return new(allocate()) ValueNodeT(*this);
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline _CmprValueNode<T,Sz,Next,Alloc>*
_CmprValueNode<T,Sz,Next,Alloc>::allocate()
{
  typename Alloc::template rebind<ValueNodeT>::other alloc;
  return alloc.allocate(1);
}

template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
inline void
_CmprValueNode<T,Sz,Next,Alloc>::destroy()
{
  typename Alloc::template rebind<ValueNodeT>::other alloc;
  this->~_CmprValueNode<T,Sz,Next,Alloc>();
  alloc.deallocate(this, 1);
}

} // end namespace ctrie
#endif
