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

#if !defined(VITA_REAL_PRIMITIVE_H)
#define      VITA_REAL_PRIMITIVE_H

#include <string>

#include "kernel/function.h"
#include "kernel/interpreter.h"
#include "kernel/random.h"
#include "kernel/terminal.h"
#include "kernel/utility.h"
#include "kernel/src/primitive/comp_penalty.h"

namespace vita
{
/// We assume that errors during floating-point operations aren't terminal
/// error. So we dont't try to prevent domain errors (e.g. square root of a
/// negative number) or range error (e.g. pow(10.0, 1e6)) checking arguments
/// beforehand (domain errors could be prevented by carefully bounds checking
/// the arguments before calling functions and taking alternative action if
/// the bounds are violated; range errors usually cannot be prevented, as
/// they are dependent on the implementation of floating-point numbers, as
/// well as the function being applied).
/// Instead we detect them and take alternative action (usually returning
/// an empty value).
namespace real
{
using base_t = double;

static_assert(std::numeric_limits<base_t>::is_iec559,
              "Vita requires IEC 559/IEEE 754 floating-point types");

///
/// \param[in] v the value that must be casted to base type (`base_t`).
/// \return the content of `v`.
///
/// Just a simple shortcut.
///
inline base_t base(const any &v) { return any_cast<base_t>(v); }

///
/// \brief Ephemeral random constant
///
/// It is assumed that the creation of floating-point constants is
/// necessary to do symbolic regression in evolutionary computation.
/// Genetic programming solves the problem of constant creation by using a
/// special terminal named "ephemeral random constant" (Koza 1992). For
/// each ephemeral random constant used in the initial population, a random
/// number of a special data type in a specified range is generated. Then
/// these random constants are moved around from genome to genome by the
/// crossover operator.
///
class real : public terminal
{
public:
  explicit real(const cvect &c, base_t m = -1000.0, base_t u = 1000.0)
    : terminal("REAL", c[0]), min(m), upp(u)
  {
    assert(c.size() == 1);
    assert(m < u);
  }

  virtual bool parametric() const override { return true; }

  virtual double init() const override
  { return random::between<base_t>(min, upp); }

  virtual std::string display(double v) const override
  { return std::to_string(v); }

  virtual any eval(core_interpreter *i) const override
  {
    return any(static_cast<base_t>(
                 any_cast<gene::param_type>(
                   static_cast<interpreter<i_mep> *>(i)->fetch_param())));
  }

private:  // Private data members
  const base_t min, upp;
};

///
/// \brief Ephemeral random integer constant
///
/// This is like real::real but restricted to integer numbers.
///
class integer : public terminal
{
public:
  explicit integer(const cvect &c, int m = -128, int u = 127)
    : terminal("REAL", c[0]), min(m), upp(u)
  {
    assert(c.size() == 1);
    assert(m < u);
  }

  virtual bool parametric() const override { return true; }

  virtual double init() const override
  { return random::between<int>(min, upp); }

  virtual std::string display(double v) const override
  { return std::to_string(static_cast<int>(v)); }

  virtual any eval(core_interpreter *i) const override
  {
    return any(static_cast<base_t>(
                 any_cast<gene::param_type>(
                   static_cast<interpreter<i_mep> *>(i)->fetch_param())));
  }

private: // Private data members
  const int min, upp;
};

///
/// \brief The absolute value of a real number
///
class abs : public function
{
public:
  explicit abs(const cvect &c) : function("FABS", c[0], {c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *i) const override
  {
    const any a(static_cast<interpreter<i_mep> *>(i)->fetch_arg(0));

    return a.empty() ? a : any(std::fabs(base(a)));
  }
};

///
/// \brief Sum of two real numbers
///
class add : public function
{
public:
  explicit add(const cvect &c) : function("FADD", c[0], {c[0], c[0]}) {}

  virtual bool associative() const override { return true; }

  virtual any eval(core_interpreter *ci) const override
  {
    auto i(static_cast<interpreter<i_mep> *>(ci));

    const any a0(i->fetch_arg(0));
    if (a0.empty())  return a0;

    const any a1(i->fetch_arg(1));
    if (a1.empty())  return a1;

    const base_t ret(base(a0) + base(a1));
    if (std::isinf(ret))  return any();

    return any(ret);
  }
};

///
/// Division between two real numbers
///
class div : public function
{
public:
  explicit div(const cvect &c) : function("FDIV", c[0], {c[0], c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto i(static_cast<interpreter<i_mep> *>(ci));

    const any a0(i->fetch_arg(0));
    if (a0.empty())  return a0;

    const any a1(i->fetch_arg(1));
    if (a1.empty())  return a1;

    const base_t ret(base(a0) / base(a1));
    if (!std::isfinite(ret))  return any();

    return any(ret);
  }
};

///
/// \brief "Greater Then" operator
///
class gt : public function
{
public:
  explicit gt(const cvect &c)
    : function(">", c[1], {c[0], c[0]})
  { assert(c.size() == 2); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto i(static_cast<interpreter<i_mep> *>(ci));
    return any(std::isgreater(base(i->fetch_arg(0)), base(i->fetch_arg(1))));
    // If one or both arguments of isless are Nan, the function returns
    // false, but no FE_INVALID exception is raised (note that the
    // expression v0 < v1 may raise an exception in this case).
  }
};

///
/// \brief Quotient of the division between two real numbers
///
class idiv : public function
{
public:
  explicit idiv(const cvect &c) : function("FIDIV", c[0], {c[0], c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto i(static_cast<interpreter<i_mep> *>(ci));

    const any a0(i->fetch_arg(0));
    if (a0.empty())  return a0;

    const any a1(i->fetch_arg(1));
    if (a1.empty())  return a1;

    const base_t ret(std::floor(base(a0) / base(a1)));
    if (!std::isfinite(ret))  return any();

    return any(ret);
  }
};

///
/// \brief "If between" operator
///
/// \warning Requires five input arguments.
///
class ifb : public function
{
public:
  explicit ifb(const cvect &c)
    : function("FIFB", c[1], {c[0], c[0], c[0], c[1], c[1]})
  { assert(c.size() == 2); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto i(static_cast<interpreter<i_mep> *>(ci));

    const any a0(i->fetch_arg(0));
    if (a0.empty())  return a0;

    const any a1(i->fetch_arg(1));
    if (a1.empty())  return a1;

    const any a2(i->fetch_arg(2));
    if (a2.empty())  return a2;

    const auto v0(base(a0));
    const auto v1(base(a1));
    const auto v2(base(a2));

    const auto min(std::fmin(v1, v2));
    const auto max(std::fmax(v1, v2));

    if (min <= v0 && v0 <= max)
      return i->fetch_arg(3);
    else
      return i->fetch_arg(4);
  }
};

///
/// \brief "If equal" operator
///
class ife : public function
{
public:
  explicit ife(const cvect &c)
    : function("FIFE", c[1], {c[0], c[0], c[1], c[1]})
  { assert(c.size() == 2); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto i(static_cast<interpreter<i_mep> *>(ci));

    const any a0(i->fetch_arg(0));
    if (a0.empty())  return a0;

    const any a1(i->fetch_arg(1));
    if (a1.empty())  return a1;

    if (issmall(base(a0) - base(a1)))
      return i->fetch_arg(2);
    else
      return i->fetch_arg(3);
  }

  virtual double penalty_nvi(core_interpreter *ci) const override
  {
    return comparison_function_penalty(ci);
  }
};

///
/// \brief "If less then" operator
///
class ifl : public function
{
public:
  explicit ifl(const cvect &c)
    : function("FIFL", c[1], {c[0], c[0], c[1], c[1]})
  { assert(c.size() == 2); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto i(static_cast<interpreter<i_mep> *>(ci));

    const any a0(i->fetch_arg(0));
    if (a0.empty())  return a0;

    const any a1(i->fetch_arg(1));
    if (a1.empty())  return a1;

    const auto v0(base(a0)), v1(base(a1));
    if (v0 < v1)
      return i->fetch_arg(2);
    else
      return i->fetch_arg(3);
  }

  virtual double penalty_nvi(core_interpreter *ci) const override
  {
    return comparison_function_penalty(ci);
  }
};

///
/// \brief "If zero" operator
///
class ifz : public function
{
public:
  explicit ifz(const cvect &c) : function("FIFZ", c[0], {c[0], c[0], c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto i(static_cast<interpreter<i_mep> *>(ci));

    const any a0(i->fetch_arg(0));
    if (a0.empty())  return a0;

    if (issmall(base(a0)))
      return i->fetch_arg(1);
    else
      return i->fetch_arg(2);
  }

  virtual double penalty_nvi(core_interpreter *ci) const override
  {
    return comparison_function_penalty(ci);
  }
};

///
/// \brief Length of a string
///
class length : public function
{
public:
  explicit length(const cvect &c) : function("FLENGTH", c[1], {c[0]})
  { assert(c.size() == 2); }

  virtual any eval(core_interpreter *i) const override
  {
    const any a(static_cast<interpreter<i_mep> *>(i)->fetch_arg(0));
    if (a.empty())  return a;

    return any(static_cast<base_t>(any_cast<std::string>(a).size()));
  }
};

///
/// \brief Natural logarithm of a real number
///
class ln : public function
{
public:
  explicit ln(const cvect &c) : function("FLN", c[0], {c[0]})
  { weight = k_base_weight / 2; }

  ///
  /// \param[in] i pointer to the active interpreter.
  /// \return the natural logarithm of its argument or an empty value in case
  //          of invalid argument / infinite result.
  ///
  virtual any eval(core_interpreter *i) const override
  {
    const any a0(static_cast<interpreter<i_mep> *>(i)->fetch_arg(0));
    if (a0.empty())  return a0;

    const base_t ret(std::log(base(a0)));
    if (!std::isfinite(ret))  return any();

    return any(ret);
  }
};

///
/// \brief "Less Then" operator
///
class lt : public function
{
public:
  explicit lt(const cvect &c)
    : function("<", c[1], {c[0], c[0]})
  { assert(c.size() == 2); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto i(static_cast<interpreter<i_mep> *>(ci));
    return any(std::isless(base(i->fetch_arg(0)), base(i->fetch_arg(1))));
    // If one or both arguments of isless are Nan, the function returns
    // false, but no FE_INVALID exception is raised (note that the
    // expression v0 < v1 may raise an exception in this case).
  }
};

///
/// \brief The larger of two floating point values
///
class max : public function
{
public:
  explicit max(const cvect &c) : function("FMAX", c[0], {c[0], c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto i(static_cast<interpreter<i_mep> *>(ci));

    const any a0(i->fetch_arg(0));
    if (a0.empty())  return a0;

    const any a1(i->fetch_arg(1));
    if (a1.empty())  return a1;

    const base_t ret(std::fmax(base(a0), base(a1)));
    if (!std::isfinite(ret))  return any();

    return any(ret);
  }
};

///
/// \brief Remainder of the division between real numbers
///
class mod : public function
{
public:
  explicit mod(const cvect &c) : function("FMOD", c[0], {c[0], c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto i(static_cast<interpreter<i_mep> *>(ci));

    const any a0(i->fetch_arg(0));
    if (a0.empty())  return a0;

    const any a1(i->fetch_arg(1));
    if (a1.empty())  return a1;

    const base_t ret(std::fmod(base(a0), base(a1)));
    if (!std::isfinite(ret))  return any();

    return any(ret);
  }
};

///
/// \brief Product of real numbers
///
class mul : public function
{
public:
  explicit mul(const cvect &c) : function("FMUL", c[0], {c[0], c[0]}) {}

  virtual any eval(core_interpreter *ci) const override
  {
    auto i(static_cast<interpreter<i_mep> *>(ci));

    const any a0(i->fetch_arg(0));
    if (a0.empty())  return a0;

    const any a1(i->fetch_arg(1));
    if (a1.empty())  return a1;

    const base_t ret(base(a0) * base(a1));
    if (std::isinf(ret))  return any();

    return any(ret);
  }
};

///
/// \brief sin() of a real number
///
class sin : public function
{
public:
  explicit sin(const cvect &c) : function("FSIN", c[0], {c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *i) const override
  {
    const any a(static_cast<interpreter<i_mep> *>(i)->fetch_arg(0));
    if (a.empty())  return a;

    return any(std::sin(base(a)));
  }
};

///
/// \brief square root of a real number
///
class sqrt : public function
{
public:
  explicit sqrt(const cvect &c) : function("FSQRT", c[0], {c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *i) const override
  {
    const any a(static_cast<interpreter<i_mep> *>(i)->fetch_arg(0));
    if (a.empty())  return a;

    const auto v(base(a));
    if (std::isless(v, 0.0))
      return any();

    return any(std::sqrt(v));
  }
};

///
/// \brief Subtraction between real numbers
///
class sub : public function
{
public:
  explicit sub(const cvect &c) : function("FSUB", c[0], {c[0], c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto i(static_cast<interpreter<i_mep> *>(ci));

    const any a0(i->fetch_arg(0));
    if (a0.empty())  return a0;

    const any a1(i->fetch_arg(1));
    if (a1.empty())  return a1;

    const base_t ret(base(a0) - base(a1));
    if (std::isinf(ret))  return any();

    return any(ret);
  }
};

}  // namespace real
}  // namespace vita

#endif  // Include guard