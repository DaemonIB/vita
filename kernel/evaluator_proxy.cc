/**
 *
 *  \file evaluator_proxy.cc
 *
 *  \author Manlio Morini
 *  \date 2011/01/08
 *
 *  This file is part of VITA
 *
 */
  
#include "evaluator_proxy.h"
#include "individual.h"

namespace vita
{

  ///
  /// \param[in] eva pointer that lets the proxy access the real evaluator.
  /// \param[in] ts 2^\a ts is the number of elements of the cache.
  ///
  evaluator_proxy::evaluator_proxy(evaluator *const eva, unsigned ts) 
    : _eva(eva), _cache(ts)
  {
    assert(eva && ts);
  }

  ///
  /// \param[in] ind the individual whose fitness we want to know.
  /// \return the fitness of \a ind.
  ///
  fitness_t
  evaluator_proxy::run(const individual &ind)
  {
    fitness_t f;
    if (!_cache.find(ind,&f))
    {
      f = _eva->run(ind);

      _cache.insert(ind,f);

#if !defined(NDEBUG)
      fitness_t f1;
      assert(_cache.find(ind,&f1) && f==f1);
#endif
    }
    /*
    #if !defined(NDEBUG)
    // Hash collision checking code can slow down the program very much.
    else
    {
      const fitness_t f1(_eva->run(ind));
      if (f != f1)
	std::cerr << "********* COLLISION *********" << std::endl;
    }
    #endif
    */

    return f;
  }

  ///
  /// Resets the evaluation caches.
  ///
  void
  evaluator_proxy::clear()
  {
    _cache.clear();
  }

  ///
  /// \return number of probes in the transposition table.
  ///
  boost::uint64_t
  evaluator_proxy::probes() const
  {
    return _cache.probes();
  }

  ///
  /// \return number of transposition table hits.
  ///
  boost::uint64_t
  evaluator_proxy::hits() const
  {
    return _cache.hits();
  }

}  // namespace vita
