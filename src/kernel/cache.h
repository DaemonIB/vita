/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_CACHE_H)
#define      VITA_CACHE_H

#include "kernel/cache_hash.h"
#include "kernel/environment.h"

namespace vita
{
///
/// Implements a hash table that links individuals' signature to fitness
/// (mainly used by the evaluator_proxy class).
///
/// During the evolution semantically equivalent (but syntactically distinct)
/// individuals are often generated and cache can give a significant speed
/// improvement avoiding the recalculation of shared information.
///
class cache
{
public:
  DISALLOW_COPY_AND_ASSIGN(cache);

  explicit cache(std::uint8_t);

  void clear();
  void clear(const hash_t &);

  void insert(const hash_t &, const fitness_t &);

  const fitness_t &find(const hash_t &) const;

  /// \return number of searches in the hash table
  /// \note Every call to the find method increment the counter.
  std::uintmax_t probes() const { return probes_; }

  /// \return number of successful searches in the hash table
  std::uintmax_t hits() const { return hits_; }

  bool debug() const;

  // Serialization.
  bool load(std::istream &);
  bool save(std::ostream &) const;

private:
  // Private support methods.
  std::size_t index(const hash_t &) const;

  // Private data members.
  struct slot
  {
    /// This is used as primary key for access to the table.
    hash_t       hash;
    /// The stored fitness of an individual.
    fitness_t fitness;
    /// Valid slots are recognized comparing their seal with the current one.
    unsigned     seal;
  };

  const std::uint64_t k_mask;
  std::vector<slot>   table_;

  decltype(slot::seal) seal_;

  mutable std::uintmax_t probes_;
  mutable std::uintmax_t   hits_;
};

/// \example example4.cc
/// Performs a speed test on the transposition table (insert-find cycle).
}  // namespace vita

#endif  // include guard
