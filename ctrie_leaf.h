#ifndef _CTRIE_LEAF_H
#define _CTRIE_LEAF_H
 
namespace ctrie {

template<typename T, template<u_char> class Next, class Alloc>
class _Leaf : public _BaseNode<T,Next,Alloc> {
private:
  T mValue;

public:
  typedef _BaseNode<T,Next,Alloc> NodeT;
  typedef _Leaf<T,Next,Alloc> LeafT;

  static LeafT* create(const char* str, size_t strLen, const T& value);
  static LeafT* create(const char* str, size_t strLen, T&& value);
  NodeT* clone() const /*override*/;
  _Leaf& operator=(const _Leaf&) = delete;
  void destroy() /*override*/;

  bool hasValue() const /*override*/                {return true;}
  T& value() /*override*/                           {return mValue;}
  const T& value() const /*override*/               {return mValue;}
  T&& valueToMove() /*override*/                    {return std::move(mValue);}

  size_t size() const /*override*/                  {return 1;};
  size_t treeSize() const /*override*/              {return 1;};

protected:
  ~_Leaf()                                          {}

private:
  _Leaf(const char* str, size_t strLen, const T& value);
  _Leaf(const char* str, size_t strLen, T&& value);
  _Leaf(const _Leaf&) = default;

  static LeafT* allocate();
};

template<typename T, template<u_char> class Next, class Alloc>
inline
_Leaf<T,Next,Alloc>::_Leaf(const char* str, size_t strLen, const T& value)
  : _BaseNode<T,Next,Alloc>(str, strLen), mValue(value)
{}

template<typename T, template<u_char> class Next, class Alloc>
inline
_Leaf<T,Next,Alloc>::_Leaf(const char* str, size_t strLen, T&& value)
  : _BaseNode<T,Next,Alloc>(str, strLen), mValue(value)
{}

template<typename T, template<u_char> class Next, class Alloc>
inline _Leaf<T,Next,Alloc>*
_Leaf<T,Next,Alloc>::create(const char* str, size_t strLen, const T& value)
{
  return new(allocate()) LeafT(str, strLen, value);
}

template<typename T, template<u_char> class Next, class Alloc>
inline _Leaf<T,Next,Alloc>*
_Leaf<T,Next,Alloc>::create(const char* str, size_t strLen, T&& value)
{
  return new(allocate()) LeafT(str, strLen, value);
}

template<typename T, template<u_char> class Next, class Alloc>
inline _BaseNode<T,Next,Alloc>*
_Leaf<T,Next,Alloc>::clone() const
{
  return new(allocate()) LeafT(*this);
}

template<typename T, template<u_char> class Next, class Alloc>
inline void
_Leaf<T,Next,Alloc>::destroy()
{
  typename Alloc::template rebind<LeafT>::other alloc;
  this->~_Leaf<T,Next,Alloc>();
  alloc.deallocate(this, 1);
}

template<typename T, template<u_char> class Next, class Alloc>
inline _Leaf<T,Next,Alloc>*
_Leaf<T,Next,Alloc>::allocate()
{
  typename Alloc::template rebind<LeafT>::other alloc;
  return alloc.allocate(1);
}

} // end namespace ctrie
#endif
