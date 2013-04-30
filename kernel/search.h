/**
 *
 *  \file search.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SEARCH_H)
#define      SEARCH_H

#include <list>

#include "environment.h"

namespace vita
{
  template <class T> class distribution;
  class evolution;
  class individual;
  class problem;
  class summary;

  ///
  /// This \c class drives the evolution.
  ///
  class search
  {
  public:
    explicit search(problem *const);

    void arl(const individual &, evolution &);
    void tune_parameters();

    individual run(unsigned = 1);

    bool debug(bool) const;

  private:  // Private support methods.
    double accuracy(const individual &) const;
    void dss(unsigned) const;
    void log(const summary &, const distribution<fitness_t> &,
             const std::list<unsigned> &, unsigned, double, unsigned);
    void print_resume(bool, const fitness_t &, double) const;
    bool stop_condition(const summary &) const;

  private:  // Private data members.
    /// This is the environment actually used during the search (\a prob_->env
    /// is used for compiling \a env_ via the tune_parameters method).
    environment env_;
    problem   *prob_;
  };
}  // namespace vita

#endif  // SEARCH_H
