/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_POPULATION_INL_H)
#define      VITA_POPULATION_INL_H

///
/// \param[in] e base vita::environment.
/// \param[in] sset base vita::symbol_set.
///
/// Creates a random population (initial size \a e.individuals).
///
template<class T>
population<T>::population(const environment &e, const symbol_set &sset)
  : pop_(1)
{
  assert(e.debug(true, true));

  const auto n(e.individuals);
  pop_[0].reserve(n);

  // DO NOT CHANGE with a call to init_layer(0): when layer 0 is empty, there
  // isn't a well defined environment and init_layer doesn't work.
  for (auto i(decltype(n){0}); i < n; ++i)
    pop_[0].emplace_back(e, sset);

  assert(debug(true));
}

///
/// \param[in] l a layer of the population.
/// \param[in] e an environmnet (used for individual generation).
/// \param[in] s a symbol_set (used for individual generation).
///
/// Resets layer \a l of the population.
///
/// \warning
/// If layer \a l is nonexistent/empty the method doesn't work!
///
template<class T>
void population<T>::init_layer(unsigned l, const environment *e,
                               const symbol_set *s)
{
  assert(l < pop_.size());
  assert(pop_[l].size() || (e && s));

  if (!e)
    e = &pop_[l][0].env();
  if (!s)
    s = &pop_[l][0].sset();

  pop_[l].clear();

  const auto n(e->individuals);
  for (auto i(decltype(n){0}); i < n; ++i)
    pop_[l].emplace_back(*e, *s);
}

///
/// \return number of active layers.
///
/// \note
/// * The number of active layers is a dynamic value (it is a monotonically
///   increasing function of the generation number).
/// * Maximum number of layers (\c env().alps.layers) is a constant value
///   greater than or equal to \c layers().
///
template<class T>
unsigned population<T>::layers() const
{
  return pop_.size();
}

///
/// Add a new layer to the population.
///
/// The new layer is inserted as the lower layer and randomly initialized.
///
template<class T>
void population<T>::add_layer()
{
  assert(pop_.size());
  assert(pop_[0].size());

  const auto &e(pop_[0][0].env());
  const auto &s(pop_[0][0].sset());

  pop_.insert(pop_.begin(), layer_t());
  pop_.front().reserve(e.individuals);

  init_layer(0, &e, &s);
}

///
/// \param[in] l index of a layer.
/// \param[in] i an individual.
///
/// Add individual \a i to layer \a l.
///
template<class T>
void population<T>::add_to_layer(unsigned l, const T &i)
{
  assert(l < layers());
  pop_[l].push_back(i);
}

///
/// \param[in] l index of a layer.
///
/// Remove the last individual of layer \a l.
///
template<class T>
void population<T>::pop_from_layer(unsigned l)
{
  assert(l < layers());
  pop_[l].pop_back();
}


///
/// \param[in] c coordinates of an \a individual.
/// \return a reference to the \a individual at coordinates \a c.
///
template<class T>
T &population<T>::operator[](coord c)
{
  assert(c.layer < layers());
  assert(c.index < individuals(c.layer));
  return pop_[c.layer][c.index];
}

///
/// \param[in] c coordinates of an individual.
/// \return a constant reference to the individual at coordinates \a c.
///
template<class T>
const T &population<T>::operator[](coord c) const
{
  assert(c.layer < layers());
  assert(c.index < individuals(c.layer));
  return pop_[c.layer][c.index];
}

///
/// \param[in] l a layer.
/// \return the number of individuals in layer \a l.
///
template<class T>
unsigned population<T>::individuals(unsigned l) const
{
  assert(l < layers());
  return pop_[l].size();
}

///
/// \return the number of individuals in the population.
///
template<class T>
unsigned population<T>::individuals() const
{
  unsigned n(0);
  for (const auto &layer : pop_)
    n += layer.size();

  return n;
}

///
/// \return a constant reference to the active environment.
///
template<class T>
const environment &population<T>::env() const
{
  assert(pop_.size());     // DO NOT CHANGE with assert(layers()) => infinite
                           // loop
  assert(pop_[0].size());  // DO NOT CHANGE with assert(individuals(0)) =>
                           // infinite loop

  return pop_[0][0].env();
}

///
/// \return an iterator pointing to the first individual of the population.
///
template<class T>
typename population<T>::const_iterator population<T>::begin() const
{
  return pop_.begin();
}

///
/// \return an iterator pointing one element past the last individual of the
///         population
///
template<class T>
typename population<T>::const_iterator population<T>::end() const
{
  return pop_.end();
}

///
/// Increments the age of each individual in the population.
///
template<class T>
void population<T>::inc_age()
{
  for (auto &l : pop_)
    for (auto &i : l)
      i.inc_age();
}

///
/// \param[in] verbose if \c true prints error messages to \c std::cerr.
/// \return \c true if the object passes the internal consistency check.
///
template<class T>
bool population<T>::debug(bool verbose) const
{
  for (const auto &l : pop_)
    for (const auto &i : l)
      if (!i.debug(verbose))
        return false;

  return true;
}

///
/// \param[in] in input stream.
/// \return \c true if population was loaded correctly.
///
/// \note
/// If the load operation isn't successful the current population isn't
/// changed.
///
template<class T>
bool population<T>::load(std::istream &in)
{
  unsigned n_layers(0);
  if (!(in >> n_layers))
    return false;

  population p(env(), pop_[0][0].sset());
  p.pop_.reserve(n_layers);

  for (decltype(n_layers) l(0); l < n_layers; ++l)
  {
    unsigned n_elem(0);
    if (!(in >> n_elem))
      return false;

    for (decltype(n_elem) i(0); i < n_elem; ++i)
      if (!p[{l, i}].load(in))
        return false;
  }

  *this = p;
  return true;
}

///
/// \param[out] out output stream.
/// \return \c true if population was saved correctly.
///
template<class T>
bool population<T>::save(std::ostream &out) const
{
  out << layers() << std::endl;

  for (const auto &l : pop_)
  {
    out << l.size() << std::endl;

    for (const auto &i : l)
      i.save(out);
  }

  return out.good();
}

///
/// \param[in,out] s output \c stream.
/// \param[in] pop population to be listed.
/// \return the output \c stream.
///
template<class T>
std::ostream &operator<<(std::ostream &s, const population<T> &pop)
{
  unsigned n_layer(0);

  for (const auto &l : pop)
  {
    s << std::string(70, '-') << std::endl << "Layer " << n_layer
      << std::string(70, '-') << std::endl;

    for (const auto &i : l)
      s << i << std::endl;

    ++n_layer;
  }

  return s;
}
#endif  // Include guard
