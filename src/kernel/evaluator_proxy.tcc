/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVALUATOR_PROXY_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_EVALUATOR_PROXY_TCC)
#define      VITA_EVALUATOR_PROXY_TCC

///
/// \param[in] eva pointer that lets the proxy access the real evaluator
/// \param[in] ts  `2^ts` is the number of elements of the cache
///
template<class T, class E>
evaluator_proxy<T, E>::evaluator_proxy(E eva, unsigned ts)
  : eva_(std::move(eva)), cache_(ts)
{
  Expects(ts > 6);
}

///
/// \param[in] prg the program (individual/team) whose fitness we want to know
/// \return        the fitness of `prg`
///
template<class T, class E>
fitness_t evaluator_proxy<T, E>::operator()(const T &prg)
{
  fitness_t f(cache_.find(prg.signature()));

  if (f.size())
  {
    assert(cache_.hits());

#if defined(CLONE_SCALING)
    // Before evaluating a program, we check if clones are already present in
    // the population.
    // When the number of clones is greater than `0`, the fitness assigned to
    // the program is multiplied by a scaling factor.
    // For further details see "Evolving Assembly Programs: How Games Help
    // Microprocessor Validation" - F.Corno, E.Sanchez, G.Squillero.

    //const auto perc(static_cast<double>(cache_.seen(prg.signature())) /
    //                cache_.hits());
    //if (0.01 < perc && perc < 1.0)
    //  f -= (f * perc).abs() * 2.0;

    f -= static_cast<double>(cache_.seen(prg.signature())) / 2.0;
#endif

    // Hash collision checking code can slow down the program very much.
#if !defined(NDEBUG)
    const fitness_t f1(eva_(prg));
    if (!almost_equal(f[0], f1[0]))
      std::cerr << "********* COLLISION ********* [" << f << " != " << f1
                << "]\n";

    // In the above comparison we consider only the first component of the
    // fitness otherwise we can have false positives.
    // For example if the fitness is a 2D vector (where the first component
    // is the "score" on the training set and the second one is the effective
    // length of the program), then the following two programs:
    //
    // PROGRAM A                 PROGRAM B
    // ------------------        ------------------
    // [000] FADD 001 002        [000] FADD 001 001
    // [001] X1                  [001] X1
    // [002] X1
    //
    // have the same signature, the same stored "score" but distinct
    // effective size and so distinct fitnesses.
#endif
  }
  else  // not found in cache
  {
    f = eva_(prg);

    cache_.insert(prg.signature(), f);

#if !defined(NDEBUG)
    fitness_t f1(cache_.find(prg.signature()));
    assert(f1.size());
    assert(almost_equal(f, f1));
#endif
  }

  return f;
}

///
/// \param[in] prg the program (individual/team) whose fitness we want to know
/// \return        an approximation of the fitness of `prg`
///
template<class T, class E>
fitness_t evaluator_proxy<T, E>::fast(const T &prg)
{
  return eva_.fast(prg);
}

///
/// \param[in] in input stream
/// \return       `true` if the object loaded correctly
///
/// \warning
/// If the load operation isn't successful the current object COULD BE changed.
/// The temporary object needed to holds values from the stream conceivably is
/// too big to justify the "no change" warranty.
///
template<class T, class E>
bool evaluator_proxy<T, E>::load(std::istream &in)
{
  return eva_.load(in) && cache_.load(in);
}

///
/// \param[out] out output stream
/// \return         `true` if the object was saved correctly
///
template<class T, class E>
bool evaluator_proxy<T, E>::save(std::ostream &out) const
{
  return eva_.save(out) && cache_.save(out);
}

///
/// Resets the evaluation cache / clear the statistics.
///
/// \param[in] what what should be cleared? (all, cache, stats
///
template<class T, class E>
void evaluator_proxy<T, E>::clear(typename evaluator<T>::clear_flag what)
{
  switch (what)
  {
  case evaluator<T>::all:
  case evaluator<T>::cache:
    cache_.clear();
    break;

  case evaluator<T>::stats:
#if defined(CLONE_SCALING)
    cache_.reset_seen();
#endif
    break;
  }
}

///
/// Clears the cached informations for a specific individual.
///
/// \param[in] prg a program (individual/team)
///
template<class T, class E>
void evaluator_proxy<T, E>::clear(const T &prg)
{
  cache_.clear(prg.signature());
}

///
/// \param[in] prg a program (individual/team)
/// \return        how many times we have seen the program `prg` during the
///                current run of the evolution / the last call of the clear
///                function
///
template<class T, class E>
unsigned evaluator_proxy<T, E>::seen(const T &prg) const
{
  return cache_.seen(prg.signature());
}

///
/// \return number of cache probes / hits
///
template<class T, class E>
std::string evaluator_proxy<T, E>::info() const
{
  return
    "hits " + std::to_string(cache_.hits()) +
    ", probes " + std::to_string(cache_.probes()) +
    " (ratio " + std::to_string(cache_.hits() * 100 / cache_.probes()) + "%)";
}

///
/// \param[in] prg a program (individual/team)
/// \return        a pointer to the executable version of `prg`
///
template<class T, class E>
std::unique_ptr<lambda_f<T>> evaluator_proxy<T, E>::lambdify(
  const T &prg) const
{
  return eva_.lambdify(prg);
}

#endif  // include guard
