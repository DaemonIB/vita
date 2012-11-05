/**
 *
 *  \file adf.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(ADF_H)
#define      ADF_H

#include <string>
#include <vector>

#include "function.h"
#include "individual.h"
#include "terminal.h"

namespace vita
{
  class interpreter;

  ///
  /// Human programmers organize sequences of repeated steps into reusable
  /// components such as subroutines, functions and classes. They then
  /// repeatedly invoke these components, typically with different inputs.
  /// Reuse eliminates the need to "reinvent the wheel" every time a particular
  /// sequence of steps is needed. Reuse also makes it possible to exploit a
  /// problem's modularities, symmetries and regularities (thereby potentially
  /// accelerate the problem-solving process). This can be taken further, as
  /// programmers typically organise these components into hierarchies in which
  /// top level components call lower level ones, which call still lower levels.
  ///
  /// \c adf_core is the core of \c adt and \c adf classes (they are in a HAS-A
  /// relationship with it).
  ///
  /// Although the acronym ADF is from Koza's automatically defined functions,
  /// in Vita subroutines are created using the ARL scheme described in
  /// "Discovery of subroutines in genetic programming" - J.P. Rosca and D.H.
  /// Ballard.
  ///
  class adf_core
  {
    friend class adf;
    friend class adt;

    explicit adf_core(const individual &);

    bool check() const;

    // Serialization.
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &, unsigned);

    // Data members
    const unsigned id;
    individual   code;

    static unsigned adf_count;
  };

  ///
  /// Subroutine with arguments.
  ///
  class adf : public function
  {
  public:
    adf(const individual &, const std::vector<category_t> &, unsigned);

    boost::any eval(interpreter *) const;

    const individual &get_code() const;
    std::string display() const;

    bool auto_defined() const;

    bool check() const;

  private:  // Serialization.
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &, unsigned);

  private:  // Data member.
    adf_core core_;
  };

  ///
  /// Subroutines WITHOUT arguments (see "An Analysis of Automatic Subroutine
  /// Discovery in Genetic Programming" - A.Dessi', A.Giani, A.Starita>).
  ///
  class adt : public terminal
  {
  public:
    adt(const individual &, unsigned);

    boost::any eval(interpreter *) const;

    const individual &get_code() const;
    std::string display() const;

    bool auto_defined() const;

    bool check() const;

  private:  // Serialization.
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &, unsigned);

  private:  // Data member.
    adf_core core_;
  };

  ///
  /// \see \c boost::serialization
  ///
  template<class Archive>
  void adf_core::serialize(Archive &ar, unsigned)
  {
    ar & adf_count;
    ar & id;
    ar & code;
  }

  ///
  /// \see \c boost::serialization
  ///
  template<class Archive>
  void adf::serialize(Archive &ar, unsigned)
  {
    ar & boost::serialization::base_object<function>(*this);
    ar & core_;
  }

  ///
  /// \see \c boost::serialization
  ///
  template<class Archive>
  void adt::serialize(Archive &ar, unsigned)
  {
    ar & boost::serialization::base_object<terminal>(*this);
    ar & core_;
  }
}  // namespace vita

#endif  // ADF_H
