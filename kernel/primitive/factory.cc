/**
 *
 *  \file factory.cc
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "kernel/primitive/factory.h"
#include "kernel/primitive/int.h"
#include "kernel/primitive/double.h"
#include "kernel/primitive/string.h"

namespace vita
{
  ///
  /// \return an instance of the singleton object symbol_factory.
  ///
  symbol_factory &symbol_factory::instance()
  {
    static symbol_factory singleton;
    return singleton;
  }

  ///
  /// The factory is preloaded with a number of common symbols.
  ///
  symbol_factory::symbol_factory()
  {
    register_symbol1<dbl::abs>   ("ABS",    d_double);
    register_symbol1<dbl::add>   ("ADD",    d_double);
    register_symbol1<dbl::add>   ("+",      d_double);
    register_symbol1<dbl::div>   ("DIV",    d_double);
    register_symbol1<dbl::div>   ("/",      d_double);
    register_symbol1<dbl::idiv>  ("IDIV",   d_double);
    register_symbol2<dbl::ife>   ("IFE",    d_double);
    register_symbol2<dbl::ifl>   ("IFL",    d_double);
    register_symbol1<dbl::ifz>   ("IFZ",    d_double);
    register_symbol1<dbl::ln>    ("LN",     d_double);
    register_symbol1<dbl::mod>   ("MOD",    d_double);
    register_symbol1<dbl::mod>   ("%",      d_double);
    register_symbol1<dbl::mul>   ("MUL",    d_double);
    register_symbol1<dbl::mul>   ("*",      d_double);
    register_symbol1<dbl::number>("NUMBER", d_double);
    register_symbol1<dbl::sin>   ("SIN",    d_double);
    register_symbol1<dbl::sub>   ("SUB",    d_double);
    register_symbol1<dbl::sub>   ("-",      d_double);

    register_symbol1<integer::add>   ("ADD",    d_int);
    register_symbol1<integer::add>   ("+",      d_int);
    register_symbol1<integer::div>   ("DIV",    d_int);
    register_symbol1<integer::div>   ("/",      d_int);
    register_symbol2<integer::ife>   ("IFE",    d_int);
    register_symbol2<integer::ife>   ("IFEQ",   d_int);
    register_symbol2<integer::ifl>   ("IFL",    d_int);
    register_symbol1<integer::ifz>   ("IFZ",    d_int);
    register_symbol1<integer::mod>   ("MOD",    d_int);
    register_symbol1<integer::mod>   ("%",      d_int);
    register_symbol1<integer::mul>   ("MUL",    d_int);
    register_symbol1<integer::mul>   ("*",      d_int);
    register_symbol1<integer::number>("NUMBER", d_int);
    register_symbol1<integer::shl>   ("SHL",    d_int);
    register_symbol1<integer::sub>   ("SUB",    d_int);
    register_symbol1<integer::sub>   ("-",      d_int);

    register_symbol2<str::ife>("IFE", d_string);
  }

  ///
  /// \param[in] name name of the symbol to be created.
  /// \param[in] d domain of the symbol to be created.
  /// \param[in] c a list of categories used by the the symbol constructor.
  /// \return an abstract pointer to the created symbol.
  ///
  /// As the factory only returns an abstract pointer, the client code (which
  /// requests the object from the factory) does not know - and is not burdened
  /// by - the actual concrete type of the object which was just created.
  /// However, the type of the concrete object is known by the abstract
  /// factory via the \a name, \a d, \a c arguments.
  /// \attention If \a name is not recognized as a preregistered symbol, it is
  /// registered on the fly as a \a constant.
  /// \note
  /// \li The client code has no knowledge whatsoever of the concrete type, not
  ///     needing to include any header files or class declarations relating to
  ///     the concrete type. The client code deals only with the abstract type.
  ///     Objects of a concrete type are indeed created by the factory, but
  ///     the client code accesses such objects only through their abstract
  ///     interface.
  /// \li Adding new concrete types is done by modifying the client code to use
  ///     a different factory, a modification which is typically one line in
  ///     one file (the different factory then creates objects of a different
  ///     concrete type, but still returns a pointer of the same abstract type
  ///     as before - thus insulating the client code from change). This is
  ///     significantly easier than modifying the client code to instantiate a
  ///     new type, which would require changing every location in the code
  ///     where a new object is created (as well as making sure that all such
  ///     code locations also have knowledge of the new concrete type, by
  ///     including for instance a concrete class header file). Since all
  ///     factory objects are stored globally in a singleton object and all
  ///     client code goes through the singleton to access the proper factory
  ///     for object creation, changing factories is as easy as changing the
  ///     singleton object.
  ///
  symbol_ptr symbol_factory::make(const std::string &name, domain_t d,
                                  const std::vector<category_t> &c)
  {
    const std::string un(boost::to_upper_copy(name));
    const map_key k({un, d});

    const category_t c1(c.size() > 0 ? c[0] : 0);
    const category_t c2(c.size() > 1 ? c[1] : 0);

    const auto it1(factory1_.find(k));
    if (it1 != factory1_.end())
      return (it1->second)(c1);
    else
    {
      const auto it2(factory2_.find(k));
      if (it2 != factory2_.end())
        return (it2->second)(c1, c2);
    }

    switch (d)
    {
    case d_bool:
      return std::make_shared<constant>(boost::lexical_cast<bool>(un), c1);
    case d_double:
      return std::make_shared<constant>(boost::lexical_cast<double>(un), c1);
    case d_int:
      return std::make_shared<constant>(boost::lexical_cast<int>(un), c1);
    case d_string:
      return std::make_shared<constant>(un, c1);
    default:
      return symbol_ptr();
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
  symbol_ptr symbol_factory::make(const std::string &, domain_t d, int min,
                                  int max, category_t c)
  {
    assert(d == d_double || d == d_int);

    switch (d)
    {
    case d_double:
      return std::make_shared<dbl::number>(c, min, max);
    case d_int:
      return std::make_shared<integer::number>(c, min, max);
    default:
      return symbol_ptr();
    }
  }

  ///
  /// \param[in] name name of the symbol.
  /// \param[in] d domain of the symbol.
  /// \return number of distinct categories needed to build the symbol.
  ///
  unsigned symbol_factory::args(const std::string &name, domain_t d) const
  {
    const std::string un(boost::to_upper_copy(name));
    const auto p(factory2_.find({un, d}));

    return p == factory2_.end() ? 1 : 2;
  }

  ///
  /// \param[in] name name of the symbol.
  /// \param[in] d domain of the symbol.
  /// \return \c true if the symbol has been unregistered.
  ///
  /// Unregister the symbol from the factory.
  /// \note constants and variable aren't registered in the factory, so they
  /// cannot be unregistered.
  ///
  bool symbol_factory::unregister_symbol(const std::string &name, domain_t d)
  {
    const std::string un(boost::to_upper_copy(name));
    const map_key k{un, d};

    return factory1_.erase(k) == 1 || factory2_.erase(k) == 1;
  }
}  // namespace vita