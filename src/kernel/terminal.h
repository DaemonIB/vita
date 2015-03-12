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
  using symbol::symbol;

  virtual unsigned arity() const override;

  virtual bool debug() const override;
};

///
/// \return 0 (this is a terminal!)
///
inline unsigned terminal::arity() const
{
  return 0;
}
}  // namespace vita

#if defined(VITA_NO_LIB)
#  include "kernel/terminal.cc"
#endif

#endif  // Include guard