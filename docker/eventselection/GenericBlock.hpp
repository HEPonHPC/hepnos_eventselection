#ifndef DIYGEN_GenericBlock_HPP
#define DIYGEN_GenericBlock_HPP

// --------------- generic block definition ----------------
// B = Bounds definition
// C = config definition
// D = data definition

template <typename B, typename C, typename D>
class GenericBlock {
public:
  typedef B bounds_type;
  typedef C config_type;
  typedef D data_type;
  typedef GenericBlock<B, C, D> block_type;
private:
  bounds_type bounds_;
  config_type configuration_;
  data_type data_;
  int gid_ = -1;

public:
  
  GenericBlock();
  GenericBlock(bounds_type const& b, config_type const& pc, int gid);

  int gid() const {return gid_;}
  // standard functions for block processing
  static void* create();
  static void destroy(void* b);
  static void save(const void* b, diy::BinaryBuffer& bb);
  static void load(void* b, diy::BinaryBuffer& bb);


  // accessor for configuration

  config_type&
  configuration()
  {
    return configuration_;
  };
  // Protocol for using the generic reduction function
  data_type& reduceData();
  static void reduce(data_type& a, data_type const& b);

  // add "other" into my block data
  void reduce(data_type const& other);

  // add my block data into "other"
  void altreduce(data_type& other) const;
};

template <typename B, typename C, typename D>
GenericBlock<B, C, D>::GenericBlock() : bounds_(1), configuration_()
{}

template <typename B, typename C, typename D>
GenericBlock<B, C, D>::GenericBlock(bounds_type const& b, config_type const& pc, int gid)
  : bounds_(b), configuration_(pc), gid_(gid)
{}

template <typename B, typename C, typename D>
void*
GenericBlock<B, C, D>::create()
{
  return new block_type;
}

template <typename B, typename C, typename D>
void
GenericBlock<B, C, D>::destroy(void* b)
{
  delete static_cast<block_type*>(b);
}

template <typename B, typename C, typename D>
void
GenericBlock<B, C, D>::save(const void* b, diy::BinaryBuffer& bb)
{
  block_type const* bp = static_cast<block_type const*>(b);
  diy::save(bb, bp->bounds_);
  diy::save(bb, bp->configuration_);
  diy::save(bb, bp->data_);
  diy::save(bb, bp->gid_);
}

template <typename B, typename C, typename D>
void
GenericBlock<B, C, D>::load(void* b, diy::BinaryBuffer& bb)
{
  block_type* bp = static_cast<block_type*>(b);
  diy::load(bb, bp->bounds_);
  diy::load(bb, bp->configuration_);
  diy::load(bb, bp->data_);
  diy::load(bb, bp->gid_);
}

template <typename B, typename C, typename D>
typename GenericBlock<B, C, D>::data_type&
GenericBlock<B, C, D>::reduceData()
{
  return data_;
}

template <typename B, typename C, typename D>
void
GenericBlock<B, C, D>::reduce(data_type& a, data_type const& b)
{
  accumulate_into_first(a, b);
}

template <typename B, typename C, typename D>
void
GenericBlock<B, C, D>::reduce(data_type const& other)
{
  reduce(data_, other);
}

template <typename B, typename C, typename D>
void
GenericBlock<B, C, D>::altreduce(data_type& other) const
{
  reduce(other, data_);
}

#endif
