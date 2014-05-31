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

#if !defined(VITA_INT_PRIMITIVE_H)
#define      VITA_INT_PRIMITIVE_H

#include <limits>
#include <string>

#include "kernel/function.h"
#include "kernel/interpreter.h"
#include "kernel/random.h"
#include "kernel/terminal.h"

namespace vita
{
  /// Integer overflow is undefined behavior. This means that implementations
  /// have a great deal of latitude in how they deal with signed integer
  /// overflow. An implementation that defines signed integer types as being
  /// modulo, for example, need not detect integer overflow. Implementations
  /// may also trap on signed arithmetic overflows, or simply assume that
  /// overflows will never happen and generate object code accordingly. For
  /// these reasons, it is important to ensure that operations on signed
  /// integers do no result in signed overflow
  namespace integer
  {
    typedef int base_t;

    ///
    /// \param[in] v the value that must be casted to base type (\c base_t).
    ///
    /// Just a simple shortcut.
    inline
    base_t cast(const any &v) { return any_cast<base_t>(v); }

    ///
    /// Integer ephemeral random constant.
    /// \see dbl::number
    ///
    class number : public terminal
    {
    public:
      explicit number(category_t t, int m = -128, int u = 127)
        : terminal("INT", t), min(m), upp(u)
      { assert(m < u); parametric_ = true; }

      virtual double init() const override
      { return random::between<int>(min, upp); }

      virtual std::string display(double v) const override
      { return boost::lexical_cast<std::string>(v); }

      virtual any eval(interpreter<i_mep> *i) const override
      { return any(static_cast<base_t>(
                     any_cast<decltype(gene::par)>(i->fetch_param()))); }

    private:  // Private data members.
      const int min, upp;
    };

    /// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Addition
    class add : public function
    {
    public:
      explicit add(category_t t) : function("ADD", t, {t, t})
      { associative_ = true; }

      any eval(interpreter<i_mep> *i) const
      {
        const auto v0(integer::cast(i->fetch_arg(0)));
        const auto v1(integer::cast(i->fetch_arg(1)));

        if (v0 > 0 && v1 > 0 && (v0 > std::numeric_limits<base_t>::max() - v1))
          return any(std::numeric_limits<base_t>::max());
        if (v0 < 0 && v1 < 0 && (v0 < std::numeric_limits<base_t>::min() - v1))
          return any(std::numeric_limits<base_t>::min());

        return any(v0 + v1);
      }
    };

    /// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Division
    class div : public function
    {
    public:
      explicit div(category_t t) : function("DIV", t, {t, t}) {}

      any eval(interpreter<i_mep> *i) const
      {
        const auto v0(integer::cast(i->fetch_arg(0)));
        const auto v1(integer::cast(i->fetch_arg(1)));

        if (v1 == 0 ||
            (v0 == std::numeric_limits<base_t>::min() && (v1 == -1)))
          return any(v0);
        else
          return any(v0 / v1);
      }
    };

    class ife : public function
    {
    public:
      explicit ife(category_t t1, category_t t2)
        : function("IFE", t2, {t1, t1, t2, t2}) {}

      any eval(interpreter<i_mep> *i) const
      {
        const auto v0(integer::cast(i->fetch_arg(0)));
        const auto v1(integer::cast(i->fetch_arg(1)));

        if (v0 == v1)
          return i->fetch_arg(2);
        else
          return i->fetch_arg(3);
      }
    };

    class ifl : public function
    {
    public:
      explicit ifl(category_t t1, category_t t2)
        : function("IFL", t2, {t1, t1, t2, t2}) {}

      any eval(interpreter<i_mep> *i) const
      {
        const auto v0(integer::cast(i->fetch_arg(0)));
        const auto v1(integer::cast(i->fetch_arg(1)));

        if (v0 < v1)
          return i->fetch_arg(2);
        else
          return i->fetch_arg(3);
      }
    };

    class ifz : public function
    {
    public:
      explicit ifz(category_t t) : function("IFZ", t, {t, t, t}) {}

      any eval(interpreter<i_mep> *i) const
      {
        const auto v0(integer::cast(i->fetch_arg(0)));

        if (v0 == 0)
          return i->fetch_arg(1);
        else
          return i->fetch_arg(2);
      }
    };

    /// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Modulo
    class mod : public function
    {
    public:
      explicit mod(category_t t) : function("MOD", t, {t, t}) {}

      any eval(interpreter<i_mep> *i) const
      {
        const auto v0(integer::cast(i->fetch_arg(0)));
        const auto v1(integer::cast(i->fetch_arg(1)));

        if (v1 == 0 ||
            (v0 == std::numeric_limits<base_t>::min() && (v1 == -1)))
          return any(v1);
        else
          return any(v0 % v1);
      }
    };

    /// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Multiplication
    class mul : public function
    {
    public:
      explicit mul(category_t t) : function("MUL", t, {t, t})
      { associative_ = true; }

      any eval(interpreter<i_mep> *i) const
      {
        static_assert(sizeof(long long) >= 2 * sizeof(base_t),
                      "Unable to detect overflow after multiplication");

        const auto v0(integer::cast(i->fetch_arg(0)));
        const auto v1(integer::cast(i->fetch_arg(1)));

        long long tmp(v0 * v1);
        if (tmp > std::numeric_limits<base_t>::max())
          return any(std::numeric_limits<base_t>::max());
        if (tmp < std::numeric_limits<base_t>::min())
          return any(std::numeric_limits<base_t>::min());

        return any(static_cast<base_t>(tmp));

        /*
        // On systems where the above relationship does not hold, the following
        // compliant solution may be used to ensure signed overflow does not
        // occur.
        if (v0 > 0)
          if (v1 > 0)
          {
            assert(v0 > 0 && v1 > 0);
            if (v0 > std::numeric_limits<base_t>::max() / v1)
              return any(std::numeric_limits<base_t>::max());
          }
          else  // v0 is positive, v1 is non-positive
          {
            assert(v0 > 0 && v1 <= 0);
            if (v1 < std::numeric_limits<base_t>::min() / v0)
              return any(std::numeric_limits<base_t>::min());
          }
        else  // v0 is non-positive
          if (v1 > 0)
          {
            assert(v0 <= 0 && v1 > 0);
            if (v0 < std::numeric_limits<base_t>::min() / v1)
              return any(std::numeric_limits<base_t>::min());
          }
          else  // v0 is non-positive, v1 is non-positive
          {
            assert(v0 <= 0 && v1 <= 0);
            if (v0 != 0 && v1 < std::numeric_limits<base_t>::max() / v0)
              return any(std::numeric_limits<base_t>::max());
          }

        return any(v0 * v1);
        */
      }
    };

    /// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-LeftShiftOperator
    class shl : public function
    {
    public:
      explicit shl(category_t t) : function("SHL", t, {t, t}) {}

      any eval(interpreter<i_mep> *i) const
      {
        const auto v0(integer::cast(i->fetch_arg(0)));
        const auto v1(integer::cast(i->fetch_arg(1)));

        if (v0 < 0 || v1 < 0 ||
            v1 >= static_cast<base_t>(sizeof(base_t) * CHAR_BIT) ||
            v0 > std::numeric_limits<base_t>::max() >> v1)
          return any(v0);

        return any(v0 << v1);
      }
    };

    /// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Subtraction
    class sub : public function
    {
    public:
      explicit sub(category_t t) : function("SUB", t, {t, t}) {}

      any eval(interpreter<i_mep> *i) const
      {
        const auto v0(integer::cast(i->fetch_arg(0)));
        const auto v1(integer::cast(i->fetch_arg(1)));

        if (v0 < 0 && v1 > 0 && (v0 < std::numeric_limits<base_t>::min() + v1))
          return any(std::numeric_limits<base_t>::min());
        if (v0 > 0 && v1 < 0 && (v0 > std::numeric_limits<base_t>::max() + v1))
          return any(std::numeric_limits<base_t>::max());

        return any(v0 - v1);
      }
    };
  }  // namespace integer
}  // namespace vita

#endif  // Include guard
