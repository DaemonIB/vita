/**
 *
 *  \file population.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(POPULATION_H)
#define      POPULATION_H

#include <fstream>
#include <vector>

#include "environment.h"

namespace vita
{
  ///
  /// \brief Holds the coordinates of an individual in a population.
  ///
  struct coord
  {
    unsigned layer;
    unsigned index;

    bool operator==(coord c) const
    { return layer == c.layer && index == c.index; }
  };

  ///
  /// \brief A group of individuals which may interact together (for example by
  ///        mating) producing offspring.
  ///
  /// \tparam T the type of the an individual.
  ///
  /// Typical population size in GP ranges from ten to many thousands. The
  /// population is organized in one or more layers that can interact in
  /// many ways (depending on the evolution strategy).
  ///
  template<class T>
  class population
  {
  public:
    typedef typename std::vector<std::vector<T>>::const_iterator
      const_iterator;

    explicit population(const environment &, const symbol_set &);

    T &operator[](coord);
    const T &operator[](coord) const;

    const_iterator begin() const;
    const_iterator end() const;

    unsigned individuals() const;
    unsigned individuals(unsigned) const;

    void init_layer(unsigned, const environment * = 0, const symbol_set * = 0);
    void add_layer();
    unsigned layers() const;
    void inc_age();
    unsigned max_age(unsigned) const;
    bool aged(coord) const;
    void add_to_layer(unsigned, const T &);

    const environment &env() const;

    bool debug(bool) const;

  public:   // Serialization.
    bool load(std::istream &);
    bool save(std::ostream &) const;

  private:  // Private data members.
    std::vector<std::vector<T>> pop_;
  };

#include "population_inl.h"

  ///
  /// \example example2.cc
  /// Creates a random population and shows its content.
  ///
}  // namespace vita

#endif  // POPULATION_H
