/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/ga/i_de.h"
#include "kernel/cache_hash.h"
#include "kernel/log.h"

namespace vita
{
///
/// \param[in] e base environment.
///
/// The process that generates the initial, random expressions has to be
/// implemented so as to ensure that they do not violate the type system's
/// constraints.
///
i_de::i_de(const environment &e) : individual(), genome_(e.sset->categories())
{
  Expects(e.debug(true));
  Expects(e.sset);

  const auto cs(parameters());
  assert(cs);

  for (auto c(decltype(cs){0}); c < cs; ++c)
    genome_[c] = gene(e.sset->roulette_terminal(c));

  Ensures(debug());
}

///
/// \param[out] s output stream.
///
/// The output stream contains a graph, described in dot language
/// (http://www.graphviz.org/), of this individual.
///
void i_de::graphviz(std::ostream &s) const
{
  s << "graph {";

  for (const auto &g : genome_)
    s << "g [label=" << g << ", shape=circle];";

  s << '}';
}

///
/// \param[out] s output stream
///
/// Prints genes of the individual.
///
std::ostream &i_de::in_line(std::ostream &s) const
{
  std::copy(genome_.begin(), genome_.end(), infix_iterator<gene>(s, " "));
  return s;
}

///
/// \brief A new individual is created mutating `this`
///
/// \param[in] p probability of gene mutation.
/// \param[in] env the current environment.
/// \return number of mutations performed.
///
/// \note
/// This function is included for compatibility with GP recombination
/// strategies. Typical differential evolution GA algorithm won't use
/// this method.
///
unsigned i_de::mutation(double p, const environment &env)
{
  Expects(0.0 <= p);
  Expects(p <= 1.0);

  unsigned n(0);

  const auto ps(parameters());
  for (category_t c(0); c < ps; ++c)
    if (random::boolean(p))
    {
      const gene g(env.sset->roulette_terminal(c));

      if (g != genome_[c])
      {
        ++n;
        genome_[c] = g;
      }
    }

  // Here we assume that a micromutation of a terminal isn't significative.
  // It can happen (the probability is very very low but...) that
  //
  // {1.0, 0, 0} --MUTATION--> {0.999999999999999999999999999999999999, 0, 0}
  //
  // The individuals are considered equal (the comparison between parametric
  // terminals are based on the `almost_equal` function) so this doesn't count
  // as mutation.
  if (n)
    signature_ = hash();

  Ensures(debug());
  return n;
}

///
/// \brief Differential evolution crossover.
/// \param[in] p crossover probability.
/// \param[in] f scaling factor range (`environment.de.weight`).
/// \param[in] a first parent.
/// \param[in] b second parent.
/// \param[in] c third parent (base vector).
/// \return the offspring (trial vector).
///
/// The offspring, also called trial vector, is generated as follows:
///
///     offspring = crossover(this, c + F * (a - b))
///
/// first the search direction is defined by calculating a
/// _difference vector_ between the pair of vectors `a` and `b` (usually
/// choosen at random from the population). This difference vector is scaled by
/// using the _scale factor_ `f`. This scaled difference vector is then added
/// to a third vector `c`, called the _base vector_. As a result a new vector
/// is obtained, known as the _mutant vector_. The mutant vector is recombined,
/// based on a used defined parameter, called _crossover probability_, with the
/// target vector `this` (also called _parent vector_).
///
/// This way no separate probability distribution has to be used which makes
/// the scheme completely self-organizing.
///
/// `a` and `b` are used for mutation, `this` and `c` for crossover.
///
i_de i_de::crossover(double p, const double f[2],
                     const i_de &a, const i_de &b, const i_de &c) const
{
  Expects(0.0 <= p && p <= 1.0);
  Expects(a.debug());
  Expects(b.debug());
  Expects(c.debug());

  const auto ps(parameters());
  Expects(ps == a.parameters());
  Expects(ps == b.parameters());
  Expects(ps == c.parameters());

  // The wighting factor is randomly selected from an interval for each
  // difference vector (a technique called dither). Dither improves convergence
  // behaviour significantly, especially for noisy objective functions.
  const auto rf(random::between(f[0], f[1]));

  i_de ret(c);

  for (auto i(decltype(ps){0}); i < ps - 1; ++i)
    if (random::boolean(p))
      ret[i] += rf * (a[i] - b[i]);
    else
      ret[i] = operator[](i);
  ret[ps - 1] += rf * (a[ps - 1] - b[ps - 1]);

  ret.set_older_age(std::max({age(), a.age(), b.age()}));

  ret.signature_.clear();
  Ensures(ret.debug());
  return ret;
}

///
/// \return the signature of this individual.
///
/// Identical individuals at genotypic level have the same signature
///
hash_t i_de::signature() const
{
  if (signature_.empty())
    signature_ = hash();

  return signature_;
}

///
/// \return the signature of this individual.
///
/// Converts this individual in a packed byte level representation and performs
/// the _MurmurHash3_ algorithm on it.
///
hash_t i_de::hash() const
{
  // From an individual to a packed byte stream...
  thread_local std::vector<unsigned char> packed;

  packed.clear();
  pack(&packed);

  /// ... and from a packed byte stream to a signature...

  // Length in bytes.
  const auto len(static_cast<unsigned>(packed.size() * sizeof(packed[0])));

  return vita::hash(packed.data(), len, 1973);
}

///
/// \param[out] p byte stream compacted version of the gene sequence.
///
void i_de::pack(std::vector<unsigned char> *const p) const
{
  for (const auto &g : genome_)
  {
    // Although 16 bit are enough to contain opcodes and parameters, they are
    // usually stored in unsigned variables (i.e. 32 or 64 bit) for
    // performance reasons.
    // Anyway before hashing opcodes/parameters we convert them to 16 bit
    // types to avoid hashing more than necessary.
    const auto opcode(static_cast<std::uint16_t>(g.sym->opcode()));
    assert(g.sym->opcode() <= std::numeric_limits<opcode_t>::max());

    // DO NOT CHANGE reinterpret_cast type to std::uint8_t since even if
    // std::uint8_t has the exact same size and representation as
    // unsigned char, if the implementation made it a distinct, non-character
    // type, the aliasing rules would not apply to it
    // (see http://stackoverflow.com/q/16138237/3235496)
    const auto *const s1 = reinterpret_cast<const unsigned char *>(&opcode);
    for (std::size_t i(0); i < sizeof(opcode); ++i)
      p->push_back(s1[i]);

    assert(g.sym->parametric());
    const auto param(g.par);

    auto s2 = reinterpret_cast<const unsigned char *>(&param);
    for (std::size_t i(0); i < sizeof(param); ++i)
      p->push_back(s2[i]);
  }
}

///
/// \param[in] x second term of comparison.
/// \return `true` if the two individuals are equal (symbol by symbol,
///         including introns).
///
/// \note
/// Age is not checked.
///
bool i_de::operator==(const i_de &x) const
{
  const bool eq(genome_ == x.genome_);

  assert(signature_.empty() != x.signature_.empty() ||
         (signature_ == x.signature_) == eq);

  return eq;
}

///
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparsion.
/// \return a numeric measurement of the difference between `lhs` and `rhs`
///         (taxicab / L1 distance).
///
double distance(const i_de &lhs, const i_de &rhs)
{
  Expects(lhs.parameters() == rhs.parameters());

  const auto cs(lhs.parameters());
  assert(cs);

  double d(0.0);
  for (unsigned i(0); i < cs; ++i)
    d += std::fabs(lhs[i] - rhs[i]);

  Ensures(d >= 0.0);
  return d;
}

///
/// \param[in] v input vector (a point in a multidimensional space).
/// \return a reference to `*this`.
///
/// Sets the individuals with values from `v`.
///
i_de &i_de::operator=(const std::vector<double> &v)
{
  Expects(v.size() == parameters());
  const auto ps(parameters());

  for (auto i(decltype(ps){0}); i < ps; ++i)
    operator[](i) = v[i];

  return *this;
}

///
/// \return `true` if the individual passes the internal consistency check.
///
bool i_de::debug() const
{
  if (empty())
  {
    if (!genome_.empty())
    {
      print.error("Inconsistent internal status for empty individual");
      return false;
    }

    if (!signature_.empty())
    {
      print.error("Empty individual must empty signature");
      return false;
    }

    return true;
  }

  const auto ps(parameters());

  for (auto i(decltype(ps){0}); i < ps; ++i)
  {
    if (!genome_[i].sym)
    {
      print.error("Empty symbol pointer at position ", i);
      return false;
    }

    if (!genome_[i].sym->terminal())
    {
      print.error("Not-terminal symbol at position ", i);
      return false;
    }

    if (genome_[i].sym->category() != i)
    {
      print.error("Wrong category: ", i,
                  genome_[i].sym->display(), " -> ",
                  genome_[i].sym->category(), " should be ", i);
      return false;
    }
  }

  if (!signature_.empty() && signature_ != hash())
  {
    print.error("Wrong signature: ", signature_, " should be ", hash());
    return false;
  }

  return true;
}

///
/// \param[in] e environment used to build the individual.
/// \param[in] in input stream.
/// \return `true` if the object has been loaded correctly.
///
/// \note
/// If the load operation isn't successful the current individual isn't
/// modified.
///
bool i_de::load_impl(std::istream &in, const environment &e)
{
  decltype(parameters()) sz;
  if (!(in >> sz))
    return false;

  decltype(genome_) v(sz);
  for (auto &g : v)
  {
    opcode_t opcode;
    if (!(in >> opcode))
      return false;

    gene temp;
    temp.sym = e.sset->decode(opcode);
    if (!temp.sym)
      return false;

    if (!(in >> temp.par))
      return false;

    g = temp;
  }

  genome_ = v;

  return true;
}

///
/// \param[out] out output stream.
/// \return `true` if the object has been saved correctly.
///
bool i_de::save_impl(std::ostream &out) const
{
  out << parameters() << '\n';
  for (const auto &g : genome_)
    out << g.sym->opcode() << ' ' << g.par << '\n';

  return out.good();
}

///
/// \param[out] s output stream.
/// \param[in] ind individual to print.
/// \return output stream including `ind`.
///
std::ostream &operator<<(std::ostream &s, const i_de &ind)
{
  return ind.in_line(s);
}

///
/// \return a vector of real values.
///
/// This is sweet "syntactic sugar" to manage i_de individuals as real value
/// vectors.
///
i_de::operator std::vector<double>() const
{
  const auto ps(parameters());
  std::vector<double> v(ps);

  for (auto i(decltype(ps){0}); i < ps; ++i)
    v[i] = operator[](i);

  return v;
}

}  // namespace vita