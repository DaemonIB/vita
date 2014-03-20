/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GENE_H)
#define      VITA_GENE_H

#include <vector>

#include "kernel/random.h"
#include "kernel/symbol.h"

namespace vita
{
  ///
  /// \brief A gene is a unit of heredity in a living organism.
  ///
  /// \tparam K the maximum number of arguments for a \a function.
  ///
  /// The \c class \a gene is the building block for an \a individual.
  ///
  template<unsigned K>
  class basic_gene
  {
  public:
    static constexpr decltype(K) k_args{K};

    basic_gene() {}
    explicit basic_gene(symbol *);
    basic_gene(const std::pair<symbol *, std::vector<index_t>> &);
    basic_gene(symbol *, index_t, index_t);

    bool operator==(const basic_gene<K> &) const;
    bool operator!=(const basic_gene<K> &g) const { return !(*this == g); }

  public:  // Public data members.
    symbol *sym;
    union
    {
      int               par;
      std::uint16_t args[K];
    };
  };

  template<unsigned K>
  std::ostream &operator<<(std::ostream &, const basic_gene<K> &);

  using gene = basic_gene<4>;

#include "kernel/gene_inl.h"
}  // namespace vita

#endif  // Include guard
