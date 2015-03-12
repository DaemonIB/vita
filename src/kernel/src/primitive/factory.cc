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

#include "kernel/src/primitive/factory.h"
#include "kernel/src/primitive/int.h"
#include "kernel/src/primitive/double.h"
#include "kernel/src/primitive/string.h"
#include "kernel/src/constant.h"
#include "kernel/i_mep.h"

namespace vita
{
  ///
  /// \return an instance of the singleton object symbol_factory.
  ///
  symbol_factory &symbol_factory::instance()
  {
    // The so called "static stack variable Singleton" implementation.
    static symbol_factory singleton;
    // Returning a reference removes the temptation to try and delete the
    // returned instance.
    return singleton;
  }

  ///
  /// The factory is preloaded with a number of common symbols.
  ///
  symbol_factory::symbol_factory()
  {
    register_symbol<dbl::abs>    ("FABS", 1);
    register_symbol<dbl::add>    ("FADD", 1);
    register_symbol<dbl::div>    ("FDIV", 1);
    register_symbol<dbl::idiv>   ("FIDIV", 1);
    register_symbol<dbl::ife>    ("FIFE", 2);
    register_symbol<dbl::ifl>    ("FIFL", 2);
    register_symbol<dbl::ifz>    ("FIFZ", 1);
    register_symbol<dbl::length> ("FLENGTH", 2);
    register_symbol<dbl::ln>     ("FLN", 1);
    register_symbol<dbl::max>    ("FMAX", 1);
    register_symbol<dbl::mod>    ("FMOD", 1);
    register_symbol<dbl::mul>    ("FMUL", 1);
    register_symbol<dbl::integer>("REAL", 1);
    register_symbol<dbl::sin>    ("FSIN", 1);
    register_symbol<dbl::sqrt>   ("FSQRT", 1);
    register_symbol<dbl::sub>    ("FSUB", 1);

    register_symbol<integer::add>   ("ADD", 1);
    register_symbol<integer::div>   ("DIV", 1);
    register_symbol<integer::ife>   ("IFE", 2);
    register_symbol<integer::ifl>   ("IFL", 2);
    register_symbol<integer::ifz>   ("IFZ", 1);
    register_symbol<integer::mod>   ("MOD", 1);
    register_symbol<integer::mul>   ("MUL", 1);
    register_symbol<integer::number>("INT", 1);
    register_symbol<integer::shl>   ("SHL", 1);
    register_symbol<integer::sub>   ("SUB", 1);

    register_symbol<str::ife>("SIFE", 2);
  }

  ///
  /// \param[in] s the string to be tested.
  /// \return \c the domain \a s is element of.
  ///
  domain_t find_domain(const std::string &s)
  {
    try
    {
      boost::lexical_cast<double>(s);
    }
    catch(boost::bad_lexical_cast &)  // not a number
    {
      if (s == "{TRUE}" || s == "{FALSE}")
        return domain_t::d_bool;

      return domain_t::d_string;
    }

    return s.find('.') == std::string::npos ? domain_t::d_int
                                            : domain_t::d_double;
  }

  ///
  /// \param[in] name name of the symbol to be created.
  /// \param[in] c a list of categories used by the the symbol constructor.
  /// \return an abstract pointer to the created symbol.
  ///
  /// As the factory only returns an abstract pointer, the client code (which
  /// requests the object from the factory) does not know - and is not burdened
  /// by - the actual concrete type of the object which was just created.
  /// However, the type of the concrete object is known by the abstract
  /// factory via the \a name and \a c arguments.
  ///
  /// \attention
  /// If \a name is not recognized as a preregistered symbol, it is
  /// registered on the fly as a \a constant.
  ///
  /// \note
  /// * The client code has no knowledge whatsoever of the concrete type, not
  ///   needing to include any header files or class declarations relating to
  ///   the concrete type. The client code deals only with the abstract type.
  ///   Objects of a concrete type are indeed created by the factory, but
  ///   the client code accesses such objects only through their abstract
  ///   interface.
  /// * Adding new concrete types is done by modifying the client code to use
  ///   a different factory, a modification which is typically one line in
  ///   one file (the different factory then creates objects of a different
  ///   concrete type, but still returns a pointer of the same abstract type
  ///   as before - thus insulating the client code from change). This is
  ///   significantly easier than modifying the client code to instantiate a
  ///   new type, which would require changing every location in the code
  ///   where a new object is created (as well as making sure that all such
  ///   code locations also have knowledge of the new concrete type, by
  ///   including for instance a concrete class header file). Since all
  ///   factory objects are stored globally in a singleton object and all
  ///   client code goes through the singleton to access the proper factory
  ///   for object creation, changing factories is as easy as changing the
  ///   singleton object.
  ///
  std::unique_ptr<symbol> symbol_factory::make(const std::string &name,
                                               cvect c)
  {
    assert(!name.empty());
    assert(!c.empty());

    const map_key k(boost::to_upper_copy(name));

    const auto it(factory_.find(k));
    if (it != factory_.end())
    {
      while (c.size() < it->second.n)
        c.push_back(category_t(0));

      return (it->second.f)(c);
    }

    switch (find_domain(k))
    {
    case domain_t::d_bool:
      return vita::make_unique<constant<bool>>(k, c[0]);
    case domain_t::d_double:
      return vita::make_unique<constant<double>>(k, c[0]);
    case domain_t::d_int:
      return vita::make_unique<constant<int>>(k, c[0]);
    case domain_t::d_string:
      return vita::make_unique<constant<std::string>>(name, c[0]);
    default:
      return nullptr;
    }
  }

  ///
  /// \param[in] d domain of the symbol.
  /// \param[in] min lower bound for the number value.
  /// \param[in] max upper bound for the number value.
  /// \param[in] c a category used by the symbol constructor.
  /// \return an abstract pointer to the created symbol.
  ///
  /// This is an alternative way to build a number.
  ///
  std::unique_ptr<symbol> symbol_factory::make(domain_t d, int min, int max,
                                               category_t c)
  {
    assert(d == domain_t::d_double || d == domain_t::d_int);

    switch (d)
    {
    case domain_t::d_double:
      return vita::make_unique<dbl::integer>(cvect{c}, min, max);
    case domain_t::d_int:
      return vita::make_unique<integer::number>(cvect{c}, min, max);
    default:
      return nullptr;
    }
  }

  ///
  /// \param[in] name name of the symbol.
  /// \return number of distinct categories needed to build the symbol.
  ///
  unsigned symbol_factory::args(const std::string &name) const
  {
    const map_key k(boost::to_upper_copy(name));
    const auto it(factory_.find(k));

    return it == factory_.end() ? 1 : it->second.n;
  }

  ///
  /// \param[in] name name of the symbol.
  /// \return \c true if the symbol has been unregistered.
  ///
  /// Unregister the symbol from the factory.
  /// \note constants and variable aren't registered in the factory, so they
  /// cannot be unregistered.
  ///
  bool symbol_factory::unregister_symbol(const std::string &name)
  {
    const map_key k(boost::to_upper_copy(name));

    return factory_.erase(k) == 1;
  }
}  // namespace vita