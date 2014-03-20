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

#if !defined(VITA_TERMINAL_H)
#define      VITA_TERMINAL_H

#include "kernel/symbol.h"

namespace vita
{
  ///
  /// \brief A symbol with zero-arity
  ///
  /// A terminal might be a variable (input to the program), a constant value
  /// or a function taking no arguments (e.g. move-north).
  ///
  class terminal : public symbol
  {
  public:
    terminal(const std::string &, category_t, bool = false, bool = false,
             unsigned = k_base_weight);

    ///
    /// \return \c false (terminals haven't arguments and cannot be
    ///         associative).
    ///
    virtual bool associative() const override { return false; }

    ///
    /// \return \c true if the terminal is an input variable.
    ///
    /// An input variable is a feature from the learning domain.
    ///
    bool input() const { return input_; }

    ///
    /// \return \c true if the terminal is parametric.
    ///
    virtual bool parametric() const override { return parametric_; }

    ///
    /// \return 0 (0 arguments <=> terminal).
    ///
    virtual unsigned arity() const override { return 0; }

    virtual bool debug() const override;

  private:  // Private data members
    const bool parametric_;
    const bool      input_;
  };
}  // namespace vita

#endif  // Include guard
