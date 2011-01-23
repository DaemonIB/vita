/**
 *
 *  \file individual.cc
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */

#include "adf.h"
#include "argument.h"
#include "environment.h"
#include "individual.h"
#include "random.h"
#include "symbol.h"

namespace vita
{

  ///
  /// \param[in] e base environment.
  /// \param[in] gen if true generates a random sequence of genes to initialize
  ///                the individual.
  ///
  /// 
  ///
  individual::individual(const environment &e, bool gen)
    : _best(0), _env(&e), _code(e.code_length)
  {
    assert(e.check());

    // **** Random generate initial code. ****
    if (gen)
    {
      for (unsigned i(0); i < e.code_length; ++i)
        _code[i] = gene(e.sset,i+1,e.code_length);

      assert(check());
    }
  }

  ///
  /// \param[out] last_symbol pointer to the la symbol of the compacted 
  ///                         individual.
  /// \return a new compacted individual.
  ///
  /// Create a new individual functionally equivalent to \c this but with the
  /// active symbols compacted and stored at the beginning of the code vector.
  ///
  individual
  individual::compact(unsigned *last_symbol) const
  {
    assert(size() == _env->code_length);
    individual dest(*this);

    unsigned new_line(0), old_line(_best);
    for (const_iterator it(*this); it(); ++new_line, old_line = ++it)
    {
      dest._code[new_line] = *it;

      for (unsigned i(0); i < new_line; ++i)
        for (unsigned j(0); j < dest._code[i].sym->argc(); ++j)
          if (dest._code[i].args[j] == old_line)
            dest._code[i].args[j] = new_line;
    }

    if (last_symbol && new_line)
      *last_symbol = new_line-1;

    assert( new_line==0 ||
	    (eff_size() && 0 < new_line && new_line <= dest.size()) );

    return dest;
  }

  ///
  /// \param[out] first_t pointer to the first symbol of the optimized 
  ///                     individual.
  /// \param[out] last_s pointer to the last symbol of the optimized individual.
  /// \return a new optimized individual.
  ///
  /// Create a new individual functionally equivalent to \c this but with the
  /// active functions compacted and stored at the beginning of the code vector
  /// and active terminals grouped at the end of the block.
  /// [<- Active functions ->|<- Active terminals ->|<- Introns ->]
  ///
  individual
  individual::optimize(unsigned *first_t, unsigned *last_s) const
  {
    // Step 1: compact the active symbols and put them at the beginning of the
    // code vector.
    unsigned first_terminal;
    individual source(compact(&first_terminal));

    // Step 2: reorganize the symbols so that terminals will be at the end of
    // the active symbols's block.
    assert(source._code[first_terminal].sym->terminal());
    const unsigned last_terminal(first_terminal);

    for (unsigned i(1); i < first_terminal; ++i)   // Looking for terminals.
      if (source._code[i].sym->terminal())
      {
        // We have a new terminal that should moved at the end of the active
        // symbols' block.
        unsigned found(0);
        for (unsigned j(first_terminal); j <= last_terminal && !found; ++j)
        {
          assert(source._code[j].sym->terminal());
          if (source._code[i] == source._code[j])
            found = j;
        }

        // The new terminal could be already present (because it was 
        // duplicated). We only need one terminal for each type.
        if (found)
        {
          // Move the symbols before the duplicated terminal one location
          // forward. The duplicated terminal will be overwritten.
          // We must pay attention to exactly change the arguments of the
          // moved functions.
          for (unsigned j(i); j > 0; --j)
          {
            source._code[j] = source._code[j-1];

            for (unsigned k(0); k < source._code[j].sym->argc(); ++k)
            {
              locus_t &arg(source._code[j].args[k]);
              if (arg == i)
                arg = found;
              else if (arg < i)
                ++arg;
            }
          }

          ++source._best;
        }
        else // !found
        {
          --first_terminal;

          if (first_terminal != i)
          {
            // Rearrange the arguments of the functions before the terminal 
            // that will be moved.
            for (unsigned j(source._best); j < i; ++j)
              for (unsigned k(0); k < source._code[j].sym->argc(); ++k)
              {
                locus_t &arg(source._code[j].args[k]);

                if (arg == i)
                  arg = first_terminal;
                else if (i < arg && arg <= first_terminal)
                  --arg;
              }
            
            const gene g(source._code[i]);

            // Move the symbols after the terminal one location backward. The
            // duplicated terminal will be overwritten (but we have a copy).
            // We must pay attention to exactly change the arguments of the
            // moved functions.
            for (unsigned j(i); j < first_terminal; ++j)
            {
              source._code[j] = source._code[j+1];

              for (unsigned k(0); k < source._code[j].sym->argc(); ++k)
              {
                locus_t &arg(source._code[j].args[k]);

                if (arg <= first_terminal)
                  --arg;
              }
            }
          
            source._code[first_terminal] = g;
            --i;
          }
        }
      }

    if (first_t)
      *first_t = first_terminal;
    if (last_s)
      *last_s = last_terminal;

    return source;
  }

  /**
   * normalize
   * \param norm[out]
   * \return 
   */
  unsigned
  individual::normalize(individual &norm) const
  {
    unsigned index(size());

    assert(size() == _env->code_length);
    individual dest(*_env,false);
    
    const unsigned ret(normalize(*this,0,index,dest));

    if (ret)
    {
      dest._best = index;

      norm = dest;
      assert(norm.eff_size() == norm.size()-norm._best);
    }

    return ret;
  }

  /**
   * normalize
   * \param src[in]
   * \param args[in]
   * \param dest_l[in,out]
   * \param dest[out]
   * \return
   */
  unsigned
  individual::normalize(const individual &src,
                        const std::vector<unsigned> *args,
                        unsigned &dest_l, individual &dest)
  {
    unsigned first_terminal, last_terminal;
    individual source(src.optimize(&first_terminal,&last_terminal));
    const unsigned ret(src.size() - (last_terminal - first_terminal + 1));

    // Step 1: mark the active terminal symbols.
    const unsigned cs(source.size());
    std::vector<int> ll(cs,-1);
    for (unsigned i(source._best); i <= last_terminal; ++i)
      ll[i] = i;

    assert (source._best < last_terminal);
    unsigned i(last_terminal+1);
    do 
    {
      if (!dest_l)
        return 0;
      
      --i;

      if (ll[i] >= 0)
      {
        const adf *padf = dynamic_cast<const adf *>(source._code[i].sym);
        if (padf)
        {
          std::vector<unsigned> args1(padf->argc());
          for (unsigned j(0); j < padf->argc(); ++j)
            args1[j] = ll[source._code[i].args[j]];

          if (!normalize(padf->get_code(),&args1,dest_l,dest))
            return 0;
          ll[i] = dest_l;
        }
        else  // Not ADF
        {
          const symbol *const sym = source._code[i].sym; 

          const argument *parg = dynamic_cast<const argument *>(sym);
          if (args && parg)
            ll[i] = (*args)[parg->index()];
          else
          {
            --dest_l;

            dest._code[dest_l].sym = sym;
            if (sym->parametric())
              dest._code[dest_l].par = source._code[i].par;
            else  // not parametric
              for (unsigned j(0); j < source._code[i].sym->argc(); ++j)
                dest._code[dest_l].args[j] = ll[source._code[i].args[j]];

            ll[i] = dest_l;
          }
        }
      }
    } while (i > source._best);

    return ret;
  }

  /**
   * mutation
   */
  unsigned
  individual::mutation()
  {
    unsigned n_mut(0);

    const unsigned cs(size());
    for (unsigned i(0); i < cs; ++i)
      if (random::boolean(_env->p_mutation))
      {
	++n_mut;
	_code[i] = gene(_env->sset,i+1,cs);
      }

    assert(check());

    return n_mut;
  }

  /**
   * uniform_cross
   * \param parent[in]
   */
  individual
  individual::uniform_cross(const individual &parent) const
  {
    assert(check() && parent.check());

    const unsigned cs(size());

    assert(_env->code_length==cs);
    individual offspring(*_env,false);

    for (unsigned i(0); i < cs; ++i)
      offspring._code[i] = random::boolean() ? _code[i] : parent._code[i];

    assert(offspring.check());
    return offspring;
  }

  /**
   * cross1
   * \param parent[in]
   */
  individual
  individual::cross1(const individual &parent)
  {
    assert(check() && parent.check());

    const unsigned cs(size());

    const unsigned cut(random::between<unsigned>(0,cs-1));
    
    const individual *parents[2] = {this, &parent};
    const bool base(random::boolean());

    assert(size() == _env->code_length);
    individual offspring(*_env,false);

    for (unsigned i(0); i < cut; ++i)
      offspring._code[i] = parents[base]->_code[i];
    for (unsigned i(cut); i < cs; ++i)
      offspring._code[i] = parents[!base]->_code[i];

    assert(offspring.check());
    return offspring;   
  }

  /**
   * cross2
   * \param parent[in]
   */
  individual
  individual::cross2(const individual &parent)
  {
    assert(check() && parent.check());

    const unsigned cs(size());

    const unsigned cut1(random::between<unsigned>(0,cs-1));
    const unsigned cut2(random::between<unsigned>(cut1+1,cs));

    const individual *parents[2] = {this, &parent};
    const bool base(random::boolean());

    assert(size() == _env->code_length);
    individual offspring(*_env,false);

    for (unsigned i(0); i < cut1; ++i)
      offspring._code[i] = parents[base]->_code[i];
    for (unsigned i(cut1); i < cut2; ++i)
      offspring._code[i] = parents[!base]->_code[i];
    for (unsigned i(cut2); i < cs; ++i)
      offspring._code[i] = parents[base]->_code[i];

    assert(offspring.check());
    return offspring;   
  }
  
  /**
   * blocks
   * \return
   */
  std::list<unsigned>
  individual::blocks() const
  {
    std::list<unsigned> bl;

    unsigned line(_best);
    for (const_iterator i(*this); i(); line = ++i)
      for (unsigned j(0); j < _code[line].sym->argc(); ++j)
	if ( _code[_code[line].args[j]].sym->argc() )  // At least depth 3
	{
	  bl.push_back(line);
	  break;;
	}

    return bl;
  }

  /**
   * replace
   * \param sym[in]
   * \param args[in]
   * \param line[in]
   * \return a new individual with a line replaced.
   *
   * Create a new individual obtained from 'this' replacing the original
   * symbol at line 'line' with a new one ('sym' + 'args').
   */
  individual
  individual::replace(const symbol *const sym,
                      const std::vector<unsigned> &args,
                      unsigned line) const
  {
    assert(sym);

    individual ret(*this);

    ret._code[line].sym = sym;
    for (unsigned i(0); i < args.size(); ++i)
      ret._code[line].args[i] = args[i];

    assert(ret.check());
    return ret;    
  }

  /**
   * replace
   * \param sym[in]
   * \param args[in]
   * \return a new individual with _best line replaced.
   *
   * Create a new individual obtained from 'this' replacing the original
   * symbol at line '_best' with a new one ('sym' + 'args').
   */
  individual
  individual::replace(const symbol *const sym,
                      const std::vector<unsigned> &args) const
  {
    return replace(sym,args,_best);
  }

  /**
   * destroy_block
   * \param line[in]
   * \return 
   */
  individual
  individual::destroy_block(unsigned line) const
  {
    assert(line < size() && !_code[line].sym->terminal());

    individual ret(*this);

    ret._code[line] = gene(_env->sset);

    assert(ret.check());
    return ret;    
  }
  
  /**
   * generalize
   * \param max_args[in]
   * \param positions[out]
   * \param types[out]
   */
  void
  individual::generalize(std::size_t max_args,
                         std::vector<unsigned> *const positions,
                         std::vector<symbol_t> *const types)
  {
    assert(max_args && max_args <= gene_args);

    std::vector<unsigned> terminals;

    // Step 1: mark the active terminal symbols.
    unsigned line(_best);
    for (const_iterator i(*this); i(); line = ++i)
      if (_code[line].sym->terminal())
	terminals.push_back(line);

    // Step 2: shuffle the terminals and pick elements 0..n-1.
    const unsigned n(std::min(max_args,terminals.size()));
    assert(n);

    if (n < size())
      for (unsigned j(0); j < n; ++j)
      {
	const unsigned r(random::between<unsigned>(j,terminals.size()));

	const unsigned tmp(terminals[j]);
	terminals[j] = terminals[r];
	terminals[r] = tmp;
      }
    
    // Step 3: randomly substitute n terminals with function arguments.
    for (unsigned j(0); j < n; ++j)
    {
      gene &g(_code[terminals[j]]);
      if (types)
        types->push_back(g.sym->type());
      if (positions)
        positions->push_back(terminals[j]);
      g.sym = _env->sset.arg(j);
    }

    assert(!positions || (positions->size() && positions->size() <= max_args));
    assert(!types || (types->size() && types->size() <= max_args));
    assert(!positions || !types || positions->size() == types->size());
  }

  ///
  /// \return 
  ///
  symbol_t
  individual::type() const
  {
    return _code[_best].sym->type();
  }

  /**
   * operator==
   * \param x
   */
  bool
  individual::operator==(const individual &x) const
  {
    return _code == x._code && _best == x._best;
  }

  ///
  /// \param[in] ind an individual to compare with \c this.
  /// \return a numeric measurement of the difference between \a ind and 
  /// \c this. 
  ///
  unsigned
  individual::distance(const individual &ind) const
  {  
    const unsigned cs(size());

    unsigned d(0);
    for (unsigned i(0); i < cs; ++i)
      if (_code[i] != ind._code[i])
	++d;

    return d;      
  }

  ///
  /// \param[out] p byte stream compacted version of the gene sequence
  ///               starting at locus \a idx.
  /// \param[in] idx locus in \c this individual.
  ///
  /// The generated byte stream has some interesting properties:
  /// - 
  ///
  void
  individual::pack(std::vector<boost::uint8_t> &p, unsigned idx) const
  {
    const gene &g(_code[idx]);

    const opcode_t opcode(g.sym->opcode());

    const boost::uint8_t *const s1 = (boost::uint8_t *)(&opcode);
    for (unsigned i(0); i < sizeof(opcode); ++i)
      p.push_back(s1[i]);

    if (g.sym->parametric())
    {
      const boost::uint8_t *const s2 = (boost::uint8_t *)(&g.par);
      for (unsigned i(0); i < sizeof(g.par); ++i)
	p.push_back(s2[i]);
    }
    else
      for (unsigned i(0); i < g.sym->argc(); ++i)
        pack(p,g.args[i]);
  }

  ///
  /// \param[in] packed packed byte stream representation of an individual.
  /// \param[in] idx locus starting from where unpack \c packed.  
  /// \return 
  ///
  unsigned
  individual::unpack(const std::vector<boost::uint8_t> &packed, unsigned idx)
  {
    unsigned unpacked(0);

    opcode_t opcode;
    std::memcpy(&opcode,&packed[idx],sizeof(opcode));
    unpacked += sizeof(opcode);

    gene g;
    g.sym = _env->sset.decode(opcode);

    if (g.sym->parametric())
    {
      std::memcpy(&g.par,&packed[idx+unpacked],sizeof(g.par));
      unpacked += sizeof(g.par);
    }

    _code.push_back(g);
    const unsigned base(size()-1);
    for (unsigned i(0); i < g.sym->argc(); ++i)
    {
      _code[base].args[i] = size();
      unpacked += unpack(packed,idx+unpacked);
    }

    return unpacked;
  }

  ///
  /// \return true if the individual passes the internal consistency check.
  ///
  bool
  individual::check() const
  {
    if (size() != _env->code_length)
      return false;

    bool last_is_terminal(false);
    unsigned line(_best);
    for (const_iterator it(*this); it(); line = ++it)
    {
      if (!_code[line].sym)
	return false;

      if (_code[line].sym->argc() > gene_args)
	return false;
      
      for (unsigned j(0); j < _code[line].sym->argc(); ++j)
	if (_code[line].args[j] >= size() || _code[line].args[j] <= line)
	  return false;

      last_is_terminal = _code[line].sym->terminal();
    }

    return 
      _best < size() && 
      last_is_terminal &&
      size() < (1u << 8*sizeof(locus_t)) && 
      eff_size() <= size() &&
      _env->check();
  }

  /**
   * graphviz
   * \param s[out]
   * \param id[in]
   */
  void
  individual::graphviz(std::ostream &s, const std::string &id) const
  {
    if (id.empty())
      s << "graph";
    else
      s << "subgraph " << id;
    s << " {";

    unsigned line(_best);
    for (const_iterator it(*this); it(); line = ++it)
    {
      const gene &g(*it);

      s << 'g' << line << " [label=\"" 
	<< (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display())
	<< "\"];";

      for (unsigned j(0); j < g.sym->argc(); ++j)
	s << 'g' << line << " -- g" << g.args[j] << ';';
    }

    s << '}' << std::endl;
  }

  /**
   * list
   * \param s[out]
   */
  void
  individual::list(std::ostream &s) const
  {
    const unsigned width( 1 + 
                          static_cast<unsigned>(std::log10(static_cast<double>(size()-1))) );

    unsigned line(_best);
    for (const_iterator it(*this); it(); line = ++it)
    {
      const gene &g(*it);

      s << '[' << std::setfill('0') << std::setw(width) << line << "] " 
	<< (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display());

      for (unsigned j(0); j < g.sym->argc(); ++j)
	s << ' ' << std::setw(width) << g.args[j];

      s << std::endl;
    }
  }
    
  ///
  /// \param[out] s
  /// \param[in] locus
  /// \param[in] indt
  /// \param[in] father
  ///
  /// 
  ///
  void
  individual::tree(std::ostream &s, 
		   unsigned locus, unsigned indt, unsigned father) const
  {
    const gene &g(_code[locus]);

    if (locus == father 
	|| !_code[father].sym->associative() 
	|| _code[father].sym != g.sym)
    {  
      std::string spaces(indt,' '); 
      s << spaces 
	<< (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display())
	<< std::endl;
      indt += 2;
    }
    
    const unsigned argc(g.sym->argc());
    if (argc)
      for (unsigned i(0); i < argc; ++i)
	tree(s,g.args[i],indt,locus);
  }

  ///
  /// \param[out] s
  ///
  /// 
  ///
  void
  individual::tree(std::ostream &s) const
  {
    tree(s,_best,0,_best);
  }

  /**
   * dump
   * \param s[out]
   */
  void
  individual::dump(std::ostream &s) const
  {
    const unsigned width( 1 + std::log10(size()-1) );

    for (unsigned i(0); i < size(); ++i)
    {
      const gene &g(_code[i]);

      s << '[' << std::setfill('0') << std::setw(width) << i << "] " 
	<< (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display());

      for (unsigned j(0); j < g.sym->argc(); ++j)
	s << ' ' << std::setw(width) << g.args[j];

      s << std::endl;
    }
  }

  /**
   *  operator<<
   */
  std::ostream &
  operator<<(std::ostream &s, const individual &ind)
  {
    ind.list(s);

    return s;
  }

  /**
   * const_iterator
   */
  individual::const_iterator::const_iterator(const individual &id) 
    : _ind(id), _l(id._best)
  {
    _lines.insert(_l);
  }

  ///
  unsigned
  individual::const_iterator::operator++()
  {
    if (!_lines.empty())
    {
      _lines.erase(_lines.begin());

      assert(_l < _ind._code.size());
      const gene &g(_ind._code[_l]);

      for (unsigned j(0); j < g.sym->argc(); ++j)
	_lines.insert(g.args[j]);

      _l = *_lines.begin();
    }

    return _l;
  }

}  // Namespace vita
