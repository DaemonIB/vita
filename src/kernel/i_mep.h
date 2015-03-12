/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_I_MEP_H)
#define      VITA_I_MEP_H

#include <cmath>
#include <functional>
#include <iomanip>
#include <set>

#include "kernel/function.h"
#include "kernel/gene.h"
#include "kernel/individual.h"
#include "kernel/matrix.h"

namespace vita
{
///
/// A single member of a `population`. Each individual contains a genome
/// which represents a possible solution to the task being tackled (i.e. a
/// point in the search space).
///
class i_mep : public individual
{
public:
  i_mep() : individual(), genome_(), best_(locus::npos()) {}

  i_mep(const environment &, const symbol_set &);
  i_mep(const environment &, const symbol_set &, const std::vector<gene> &);

  // Visualization/output methods
  std::ostream &dump(std::ostream &) const;
  void graphviz(std::ostream &, const std::string & = "") const;
  virtual std::ostream &in_line(std::ostream &) const override;
  std::ostream &list(std::ostream &) const;
  std::ostream &tree(std::ostream &) const;

  // Recombination operators
  unsigned mutation()
  { assert(env_->p_mutation >= 0.0); return mutation(env_->p_mutation); }
  unsigned mutation(double);
  i_mep crossover(i_mep) const;

  // Working with blocks / genome
  std::vector<locus> blocks() const;
  i_mep destroy_block(index_t) const;
  i_mep get_block(const locus &) const;

  i_mep replace(const gene &) const;
  i_mep replace(const locus &, const gene &) const;

  std::pair<i_mep, std::vector<locus>> generalize(unsigned) const;

  void set(const locus &l, const gene &g);

  // Comparison
  bool operator==(const i_mep &) const;
  unsigned distance(const i_mep &) const;

  hash_t signature() const;

  const gene &operator[](const locus &l) const;

  unsigned size() const;

  unsigned eff_size() const;

  category_t category() const;

  bool debug(bool = true) const;

  class const_iterator;
  const_iterator begin() const;
  const_iterator end() const;

  friend class interpreter<i_mep>;

private:   // NVI implementation (serialization)
  virtual bool load_nvi(std::istream &) override;
  virtual bool save_nvi(std::ostream &) const override;

private:  // Private support methods
  hash_t hash() const;
  void in_line(std::ostream &, const locus &) const;
  void pack(const locus &, std::vector<unsigned char> *const) const;
  void tree(std::ostream &, const locus &, unsigned, const locus &) const;

private:  // Private data members
  // This is the genome: the entire collection of genes (the entirety of an
  // organism's hereditary information).
  matrix<gene> genome_;

  // Starting point of the active code in this individual (the best sequence
  // of genes is starting here).
  locus best_;
};  // class i_mep

///
/// \param[in] l locus of a `gene`.
/// \return the l-th gene of `this` individual.
///
inline const gene &i_mep::operator[](const locus &l) const
{
  return genome_(l);
}

///
/// \return the total size of the individual (effective size + introns).
///
/// The size is constant for any individual (it's chosen at initialization
/// time).
/// \see eff_size()
///
inline unsigned i_mep::size() const
{
  return genome_.rows();
}

///
/// \param[in] l locus of a `gene`.
/// \param[in] g a new gene.
///
/// Set locus `l` of the genome to `g`. Please note that this is one of
/// the very few methods that aren't const.
///
inline void i_mep::set(const locus &l, const gene &g)
{
  genome_(l) = g;
  signature_.clear();
}

std::ostream &operator<<(std::ostream &, const i_mep &);

#include "kernel/i_mep_iterator.tcc"

///
/// \return an iterator to the first active locus of the individual.
///
inline i_mep::const_iterator i_mep::begin() const
{
  return i_mep::const_iterator(*this);
}

///
/// \return an iterator used as sentry value to stop a cycle.
///
inline i_mep::const_iterator i_mep::end() const
{
  return i_mep::const_iterator();
}

///
/// \example example1.cc
/// Creates a random individual and shows its content.
///
/// \example example3.cc
/// Performs three types of crossover between two random individuals.
///
}  // namespace vita

#endif  // Include guard