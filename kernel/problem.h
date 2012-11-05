/**
 *
 *  \file problem.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(PROBLEM_H)
#define      PROBLEM_H

#include <vector>

#include "data.h"
#include "environment.h"

namespace vita
{
  class evaluator;

  /// Just a shortcut.
  typedef std::shared_ptr<evaluator> evaluator_ptr;

  class problem
  {
  public:
    problem();

    unsigned add_evaluator(evaluator_ptr);
    evaluator *get_evaluator();
    void set_evaluator(unsigned);

    virtual vita::data *data() { return 0; }

    virtual void clear();
    virtual bool check(bool) const;

  public:  // Public data members.
    environment env;

  protected:  // Private data members.
    std::vector<evaluator_ptr> evaluators_;
    evaluator_ptr              active_eva_;
  };
}  // namespace vita

#endif  // PROBLEM_H
