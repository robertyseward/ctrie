#ifndef _CTRIE_FULL_H
#define _CTRIE_FULL_H
 
namespace ctrie {

template<typename T, template<u_char Sz> class Next, class Alloc>
class _FullNode : public _BaseNode<T,Next,Alloc> {
public:
  typedef _BaseNode<T,Next,Alloc> NodeT;
  typedef _Leaf<T,Next,Alloc> LeafT;
  typedef _FullNode<T,Next,Alloc> FullNodeT;
  typedef _FullValueNode<T,Next,Alloc> FullValueNodeT;

private:
  static const size_t sMaxNumChildren = 1 << (8 * sizeof(char));
  NodeT* mParent;
  char mParentIndex;
  u_char mNumChildren; // really, number of children - 1
  NodeT* mChildren[sMaxNumChildren];

public:
  static FullNodeT*
      create(NodeT* parent, const char* str, size_t strLen, char parentIndex);
  static FullNodeT* move(FullNodeT& x);
  template<u_char SrcSz>
    static FullNodeT* move(_CmprNode<T,SrcSz,Next,Alloc>& x);
  NodeT* moveAddValue(const T& valueToAdd) /*override*/;
  NodeT* clone() const /*override*/;
  void destroy() /*override*/;
  _FullNode& operator=(const _FullNode&) = delete;

  bool empty() const /*override*/                   {return false;}
  size_t size() const /*override*/                  {return mNumChildren + 1;}
  size_t treeSize() const /*override*/;
  NodeT* parent() const /*override */               {return mParent;}
  void setParent(NodeT* parent)                     {mParent = parent;}
  char parentIndex() const /*override*/             {return mParentIndex;}
  void setParentIndex(char i)/*override*/           {mParentIndex = i;}
  char key(size_t i) /*override*/                   {return (char) i;}

  NodeT** getEntryPtr(size_t i) /*override*/        {return mChildren + i;}
  NodeT* getEntry(size_t i) const /*override*/      {return mChildren[i];}
  std::pair<size_t, bool> findEntry(char key) const /*override*/;
  size_t insertEntry(NodeT* entry, size_t index, char key, NodeT** replacement)
      /*override*/;
  size_t eraseEntry(size_t index, NodeT** newNode) /*override*/;
  size_t firstEntry() const /*override*/;
  size_t lastEntry() const /*override*/;
  size_t nextEntry(size_t index) const /*override*/;
  size_t prevEntry(size_t index) const /*override*/;

protected:
  _FullNode(NodeT* parent, const char* str, size_t strLen, char parentIndex);
  _FullNode(const FullNodeT& x);
  template<u_char SrcSz>
    _FullNode(_CmprNode<T,SrcSz,Next,Alloc>&& src);
  ~_FullNode();

private:
  static FullNodeT* allocate();
  void init();

  friend class _CmprNode<T,2,Next,Alloc>;
  friend class _CmprNode<T,4,Next,Alloc>;
  friend class _CmprNode<T,8,Next,Alloc>;
  friend class _CmprNode<T,16,Next,Alloc>;
  friend class _CmprNode<T,32,Next,Alloc>;
};

template<typename T, template<u_char Sz> class Next, class Alloc>
class _FullValueNode : public _FullNode<T,Next,Alloc> {
private:
  T mValue;

public:
  typedef _BaseNode<T,Next,Alloc> NodeT;
  typedef _FullNode<T,Next,Alloc> FullNodeT;
  typedef _FullValueNode<T,Next,Alloc> ValueNodeT;

  static ValueNodeT* create(NodeT* parent, const char* str, size_t strLen,
      char parentIndex);
  template<u_char SrcSz>
    static ValueNodeT* move(_CmprValueNode<T,SrcSz,Next,Alloc>& x);
  static ValueNodeT* move(_FullNode<T,Next,Alloc>& x, const T& value);

  NodeT* clone() const /*override*/;
  NodeT* moveRemoveValue() /*override*/        {return FullNodeT::move(*this);}
  void destroy() /*override*/;
  _FullValueNode(const ValueNodeT&&) = delete;
  ValueNodeT& operator=(const ValueNodeT&) = delete;
  ValueNodeT& operator=(const ValueNodeT&&) = delete;

  bool hasValue() const /*override*/           {return true;}
  T& value() /*override*/                      {return mValue;}
  const T& value() const /*override*/          {return mValue;}
  T&& valueToMove() /*override*/               {return std::move(mValue);}

protected:
  ~_FullValueNode()                            {}

private:
  _FullValueNode(NodeT* parent, const char* str, size_t strLen,
      char parentIndex, T& value);
  _FullValueNode(const ValueNodeT& src);
  _FullValueNode(FullNodeT&& src, const T& value);
  template<u_char SrcSz>
    _FullValueNode(_CmprValueNode<T,SrcSz,Next,Alloc>&& src);

  static ValueNodeT* allocate();
};

template<typename T, template<u_char> class Next, class Alloc>
inline
_FullNode<T,Next,Alloc>::_FullNode(
    NodeT* parent, const char* str, size_t strLen, char parentIndex)
  : _BaseNode<T,Next,Alloc>(str, strLen), mParent(parent),
    mParentIndex(parentIndex), mNumChildren(0)
{
  init();
}

template<typename T, template<u_char> class Next, class Alloc>
template<u_char SrcSz>
_FullNode<T,Next,Alloc>::_FullNode(_CmprNode<T,SrcSz,Next,Alloc>&& src)
  : _BaseNode<T,Next,Alloc>(std::move(src)), mParent(0), mParentIndex(0),
     mNumChildren(0)
{
  init();
  std::swap(mParent, src.mParent);
  mNumChildren = static_cast<u_char>(src.mNumChildren - 1);
  src.mNumChildren = 0;
  mParentIndex = src.parentIndex();
  for (u_char i = 0; i <= mNumChildren; ++i) {
    mChildren[src.mCharTable[i]] = src.mChildren[i];
    if (dynamic_cast<LeafT*>(src.mChildren[i]) == 0) {
      src.mChildren[i]->setParent(this);
    }
    src.mChildren[i] = 0;
  }
}

template<typename T, template<u_char> class Next, class Alloc>
_FullNode<T,Next,Alloc>::_FullNode(const FullNodeT& x)
  : _BaseNode<T,Next,Alloc>(x), mParent(x.mParent),
    mParentIndex(x.mParentIndex), mNumChildren(x.mNumChildren)
{
  for (size_t i = 0; i < sMaxNumChildren; ++i) {
    mChildren[i] = nullptr;
    if (x.mChildren[i]) {
      mChildren[i] = x.mChildren[i]->clone();
      if (dynamic_cast<LeafT*>(mChildren[i]) == 0) {
        mChildren[i]->setParent(this);
      }
    }
  }
}

template<typename T, template<u_char> class Next, class Alloc>
_FullNode<T,Next,Alloc>::~_FullNode()
{
  for (size_t i = 0; i < sMaxNumChildren; ++i) {
    if (mChildren[i]) {
      mChildren[i]->destroy();
      mChildren[i] = nullptr;
    }
  }
}

template<typename T, template<u_char> class Next, class Alloc>
inline _FullNode<T,Next,Alloc>*
_FullNode<T,Next,Alloc>::create(
    NodeT* parent, const char* str, size_t strLen, char parentIndex)
{
  return new(allocate()) FullNodeT(parent, str, strLen, parentIndex);
}

template<typename T, template<u_char> class Next, class Alloc>
inline _FullNode<T,Next,Alloc>*
_FullNode<T,Next,Alloc>::move(FullNodeT& x)
{
  return new(allocate()) FullNodeT(std::move(x));
}

template<typename T, template<u_char> class Next, class Alloc>
template<u_char SrcSz>
inline _FullNode<T,Next,Alloc>*
_FullNode<T,Next,Alloc>::move(_CmprNode<T,SrcSz,Next,Alloc>& x)
{
  return new(allocate()) FullNodeT(std::move(x));
}

template<typename T, template<u_char> class Next, class Alloc>
inline _BaseNode<T,Next,Alloc>*
_FullNode<T,Next,Alloc>::moveAddValue(const T& valueToAdd)
{
  return FullValueNodeT::move(*this, valueToAdd);
}

template<typename T, template<u_char> class Next, class Alloc>
inline _BaseNode<T,Next,Alloc>*
_FullNode<T,Next,Alloc>::clone() const
{
  return new(allocate()) FullNodeT(*this);
}

template<typename T, template<u_char> class Next, class Alloc>
inline void
_FullNode<T,Next,Alloc>::destroy()
{
  typename Alloc::template rebind<FullNodeT>::other alloc;
  this->~_FullNode<T,Next,Alloc>();
  alloc.deallocate(this, 1);
}

template<typename T, template<u_char> class Next, class Alloc>
size_t
_FullNode<T,Next,Alloc>::treeSize() const
{
   size_t size = this->hasValue() ? 1 : 0;
   for (size_t i = 0; i < sMaxNumChildren; ++i) {
      if (mChildren[i]) {
	 size += mChildren[i]->treeSize();
      }
   }
   return size;
}

template<typename T, template<u_char> class Next, class Alloc>
inline std::pair<size_t, bool>
_FullNode<T,Next,Alloc>::findEntry(char key) const
{
  return std::make_pair(key, mChildren[static_cast<u_char>(key)] != nullptr);
}

template<typename T, template<u_char> class Next, class Alloc>
size_t
_FullNode<T,Next,Alloc>::insertEntry(
    NodeT* entry, size_t index, char key, NodeT**)
{
  assert(index < sMaxNumChildren);
  assert(mChildren[index] == nullptr);
  if (dynamic_cast<LeafT*>(entry) == 0) {
    entry->setParent(this);
    entry->setParentIndex(key);
  }
  mChildren[index] = entry;
  ++mNumChildren;
  return index;
}

template<typename T, template<u_char> class Next, class Alloc>
size_t
_FullNode<T,Next,Alloc>::eraseEntry(size_t index, NodeT** newNode)
{
  --mNumChildren;
  *newNode = this;
  size_t nextIndex = nextEntry(index);
  bool atEnd = (nextIndex == NodeT::endIndex());
  mChildren[index] = 0;
  if (mNumChildren + 1 <=
      Next<std::numeric_limits<u_char>::max()>::downThreshold) {
    FullValueNodeT *nodeWithValue = dynamic_cast<FullValueNodeT*>(this);
    if (nodeWithValue) {
      *newNode = _CmprValueNode<
          T,Next<std::numeric_limits<u_char>::max()>::down,Next,Alloc>::
          move(*nodeWithValue);
    } else {
      *newNode = _CmprNode<
          T,Next<std::numeric_limits<u_char>::max()>::down,Next,Alloc>::
          move(*this);
    }
    nextIndex = atEnd ? NodeT::endIndex() :
        (*newNode)->findEntry((char) nextIndex).first;
  }
  return nextIndex;
}

template<typename T, template<u_char> class Next, class Alloc>
inline size_t
_FullNode<T,Next,Alloc>::firstEntry() const
{
  return nextEntry(NodeT::valueIndex());
}

template<typename T, template<u_char> class Next, class Alloc>
inline size_t
_FullNode<T,Next,Alloc>::lastEntry() const
{
  return prevEntry(NodeT::endIndex());
}

template<typename T, template<u_char> class Next, class Alloc>
size_t
_FullNode<T,Next,Alloc>::nextEntry(size_t index) const
{
  index = index == NodeT::valueIndex() ? 0 : (index + 1);
  for (; index < sMaxNumChildren && mChildren[index] == nullptr; ++index) {}
  return index < sMaxNumChildren ? index : NodeT::endIndex();
}

template<typename T, template<u_char> class Next, class Alloc>
size_t
_FullNode<T,Next,Alloc>::prevEntry(size_t index) const
{
  if (index == NodeT::valueIndex() || index == 0) {
    return NodeT::valueIndex();
  }

  index = index == NodeT::endIndex() ? (sMaxNumChildren - 1) : (index - 1);
  for (; mChildren[index] == nullptr; --index) {
    if (index == 0) {
      return NodeT::valueIndex();
    }
  }
  return index;
}

template<typename T, template<u_char> class Next, class Alloc>
inline _FullNode<T,Next,Alloc>*
_FullNode<T,Next,Alloc>::allocate()
{
  typename Alloc::template rebind<FullNodeT>::other alloc;
  return alloc.allocate(1);
}

template<typename T, template<u_char> class Next, class Alloc>
inline void
_FullNode<T,Next,Alloc>::init()
{
  std::fill(mChildren, mChildren + sMaxNumChildren, nullptr);
}

template<typename T, template<u_char> class Next, class Alloc>
inline
_FullValueNode<T,Next,Alloc>::_FullValueNode(
    NodeT* parent, const char* str, size_t strLen, char parentIndex, T& value)
  : _FullNode<T,Next,Alloc>(str, strLen, parentIndex),
    mValue(value)
{}

template<typename T, template<u_char> class Next, class Alloc>
inline
_FullValueNode<T,Next,Alloc>::_FullValueNode(const ValueNodeT& src)
  : _FullNode<T,Next,Alloc>(src),
   mValue(src.value())
{}

template<typename T, template<u_char> class Next, class Alloc>
inline
_FullValueNode<T,Next,Alloc>::_FullValueNode(FullNodeT&& src, const T& value)
  : _FullNode<T,Next,Alloc>(std::move(src)),
    mValue(value)
{}

template<typename T, template<u_char> class Next, class Alloc>
template<u_char SrcSz>
inline
_FullValueNode<T,Next,Alloc>::_FullValueNode(
    _CmprValueNode<T,SrcSz,Next,Alloc>&& src)
  : _FullNode<T,Next,Alloc>(std::move(src)),
    mValue(src.valueToMove())
{}

template<typename T, template<u_char> class Next, class Alloc>
inline _FullValueNode<T,Next,Alloc>*
_FullValueNode<T,Next,Alloc>::create(
    NodeT* parent, const char* str, size_t strLen, char parentIndex)
{
  return new(allocate()) ValueNodeT(parent, str, strLen, parentIndex);
}

template<typename T, template<u_char> class Next, class Alloc>
template<u_char SrcSz>
inline _FullValueNode<T,Next,Alloc>*
_FullValueNode<T,Next,Alloc>::move(_CmprValueNode<T,SrcSz,Next,Alloc>& x)
{
  return new(allocate()) ValueNodeT(std::move(x));
}

template<typename T, template<u_char> class Next, class Alloc>
inline _FullValueNode<T,Next,Alloc>*
_FullValueNode<T,Next,Alloc>:: move(_FullNode<T,Next,Alloc>& x, const T& value)
{
  return new(allocate()) ValueNodeT(std::move(x), value);
}

template<typename T, template<u_char> class Next, class Alloc>
inline _BaseNode<T,Next,Alloc>*
_FullValueNode<T,Next,Alloc>::clone() const
{
  return new(allocate()) ValueNodeT(*this);
}

template<typename T, template<u_char> class Next, class Alloc>
inline _FullValueNode<T,Next,Alloc>*
_FullValueNode<T,Next,Alloc>::allocate()
{
  typename Alloc::template rebind<ValueNodeT>::other alloc;
  return alloc.allocate(1);
}

template<typename T, template<u_char> class Next, class Alloc>
inline void
_FullValueNode<T,Next,Alloc>::destroy()
{
  typename Alloc::template rebind<ValueNodeT>::other alloc;
  this->~_FullValueNode<T,Next,Alloc>();
  alloc.deallocate(this, 1);
}

} // end namespace ctrie
#endif
