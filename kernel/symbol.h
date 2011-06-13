/**
 *
 *  \file symbol.h
 *
 *  \author Manlio Morini
 *  \date 2009/10/14
 *
 *  This file is part of VITA
 *
 */

#if !defined(SYMBOL_H)
#define      SYMBOL_H

#include <sstream>
#include <string>

#include <boost/any.hpp>

#include "vita.h"

namespace vita
{

  const symbol_t sym_void(0);
  const symbol_t sym_bool(1);
  const symbol_t sym_real(2);
  const symbol_t free_symbol(3);

  class interpreter;

  ///
  /// GP assembles variable length program structures from basic units called
  /// functions and terminals. Functions perform operations on their inputs,
  /// which are either terminals or output from other functions.
  /// Together functions and terminals are referred to as symbols (or nodes).
  ///
  class symbol
  {
  public:
    symbol(const std::string &, symbol_t, unsigned);

    opcode_t opcode() const;
    bool terminal() const;

    symbol_t type() const;
    virtual symbol_t arg_type(unsigned) const = 0;

    virtual std::string display() const;
    virtual std::string display(int) const;
    virtual int init() const;

    virtual bool associative() const = 0;
    virtual bool parametric() const = 0;

    // The arity of a function is the number of inputs to or arguments of that
    // funtion.
    virtual unsigned arity() const = 0;

    virtual boost::any eval(interpreter *) const = 0;

    bool check() const;

    unsigned weight;

  private:
    static unsigned _opc_count;

    const opcode_t     _opcode;

    const symbol_t       _type;

    const std::string _display;
  };

  /**
   * symbol
   * \param dis[in]
   * \param w[in]
   */
  inline
  symbol::symbol(const std::string &dis, symbol_t t, unsigned w)
    : weight(w), _opcode(++_opc_count), _type(t), _display(dis)
  {
    assert(check());
  }

  ///
  /// \return 0.
  ///
  /// This function is used to initialize the symbol's internal parameter.
  /// Derived classes should redefine the init member function in a
  /// meaningful way.
  ///
  inline
  int
  symbol::init() const
  {
    return 0;
  }

  ///
  /// \return true if this symbol is a terminal.
  ///
  inline
  bool
  symbol::terminal() const
  {
    return arity() == 0;
  }

  ///
  /// \return the type of the \a symbol.
  ///
  inline
  symbol_t
  symbol::type() const
  {
    return _type;
  }

  ///
  /// \return the opcode of the symbol (an \c unsigned \c int used as primary 
  /// key).
  ///
  inline
  opcode_t
  symbol::opcode() const
  {
    return _opcode;
  }
   
}  // namespace vita

#endif  // SYMBOL_H
