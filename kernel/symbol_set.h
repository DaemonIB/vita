/**
 *
 *  \file symbol_set.h
 *
 *  \author Manlio Morini
 *  \date 2011/05/11
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(SYMBOL_SET_H)
#define      SYMBOL_SET_H

#include <string>
#include <vector>

#include "vita.h"

namespace vita
{

  class adf;
  class argument;
  class symbol;
  class terminal;
  class variable;

  class symbol_set
  {
  public:
    symbol_set();
    ~symbol_set();

    void insert(symbol *const, bool);

    const symbol *roulette(bool = false) const;
    const argument *arg(unsigned) const;
    const symbol *special(unsigned) const;
    unsigned specials() const;

    void reset_adf_weights();

    const symbol *decode(unsigned) const;
    const symbol *decode(const std::string &) const;

    void delete_symbols();

    bool check() const;

  private:
    void clear();

    std::vector<symbol *>     _symbols;
    std::vector<terminal *> _terminals;
    std::vector<adf *>            _adf;
    std::vector<argument *> _arguments;
    std::vector<terminal *>  _specials;

    unsigned long _sum;
  };
  
}  // namespace vita

#endif  // SYMBOL_SET_H
