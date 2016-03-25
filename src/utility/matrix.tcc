/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_MATRIX_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_MATRIX_TCC)
#define      VITA_MATRIX_TCC

///
/// \brief Standard `rs` x `cs` matrix. Entries aren't initialized
/// \param[in] rs number of rows.
/// \param[in] cs number of columns.
///
/// \note
/// Default values for `rs` and `cs` is `0` (i.e. uninitialized matrix,
/// sometimes we need it for performance).
///
template<class T>
matrix<T>::matrix(std::size_t rs, std::size_t cs) : data_(rs * cs), cols_(cs)
{
  assert((rs && cs) || (!rs && !cs));
}

///
/// \param[in] r row.
/// \param[in] c column.
/// \return the index in the internal vector used to store the content of the
///         matrix.
///
template<class T>
std::size_t matrix<T>::index(std::size_t r, std::size_t c) const
{
  assert(c < cols());

  return r * cols() + c;
}

///
/// \param[in] l a locus of the genome.
/// \return an element of the matrix.
///
template<class T>
typename matrix<T>::const_reference matrix<T>::operator()(const locus &l) const
{
  return data_[index(l.index, l.category)];
}

///
/// \param[in] l a locus of the genome.
/// \return an element of the matrix.
///
template<class T>
typename matrix<T>::reference matrix<T>::operator()(const locus &l)
{
  // DO NOT CHANGE THE RETURN TYPE WITH T (the method won't work for T == bool)
  return data_[index(l.index, l.category)];
}

///
/// \param[in] r row.
/// \param[in] c column.
/// \return an element of the matrix.
///
template<class T>
typename matrix<T>::const_reference matrix<T>::operator()(std::size_t r,
                                                          std::size_t c) const
{
  return data_[index(r, c)];
}

///
/// \param[in] r row.
/// \param[in] c column.
/// \return an element of the matrix.
///
template<class T>
typename matrix<T>::reference matrix<T>::operator()(std::size_t r,
                                                    std::size_t c)
{
  // DO NOT CHANGE THE RETURN TYPE WITH T (the method won't work for T == bool)
  return data_[index(r, c)];
}

///
/// \return `true` if the matrix is empty (`cols() == 0`).
///
template<class T>
bool matrix<T>::empty() const
{
  return size() == 0;
}

///
/// \return number of elements of the matrix.
///
template<class T>
std::size_t matrix<T>::size() const
{
  return data_.size();
}

///
/// \return number of rows of the matrix.
///
template<class T>
std::size_t matrix<T>::rows() const
{
  return cols() ? data_.size() / cols() : 0;
}

///
/// \return number of columns of the matrix.
///
template<class T>
std::size_t matrix<T>::cols() const
{
  return cols_;
}

///
/// \param[in] m second term of comparison.
/// \return `true` if `m` is equal to `*this`.
///
template<class T>
bool matrix<T>::operator==(const matrix &m) const
{
  return cols() == m.cols() && data_ == m.data_;
}

///
/// \param[in] v a value.
///
/// Sets the elements of the matrix to `v`.
///
template<class T>
void matrix<T>::fill(const T &v)
{
  std::fill(begin(), end(), v);
}

///
/// \return iterator to the first element of the matrix.
///
template<class T>
typename matrix<T>::iterator matrix<T>::begin()
{
  return data_.begin();
}

///
/// \return constant iterator to the first element of the matrix.
///
template<class T>
typename matrix<T>::const_iterator matrix<T>::begin() const
{
  return data_.begin();
}

///
/// \return iterator to the end (i.e. the element after the last element) of
///         the matrix.
///
template<class T>
typename matrix<T>::const_iterator matrix<T>::end() const
{
  return data_.end();
}

///
/// \return iterator to the end (i.e. the element after the last element) of
///         the matrix.
///
template<class T>
typename matrix<T>::iterator matrix<T>::end()
{
  return data_.end();
}

///
/// \param[out] out output stream.
/// \return true on success.
///
/// Saves the matrix on persistent storage.
///
/// \note
/// The method is based on `operator<<` so it works for basic `T` only.
///
template<class T>
bool matrix<T>::save(std::ostream &out) const
{
  static_assert(std::is_integral<T>::value,
                "matrix::save doesn't support non-integral types");

  out << cols() << ' ' << rows() << '\n';

  for (const auto &e : data_)
    out << e << '\n';

  return out.good();
}

///
/// \param[in] in input stream.
/// \return true on success.
///
/// Loads the matrix from persistent storage.
///
/// \note
/// * If the operation fails the object isn't modified.
/// * The method is based on `operator>>` so it works for basic `T` only.
///
template<class T>
bool matrix<T>::load(std::istream &in)
{
  static_assert(std::is_integral<T>::value,
                "matrix::load doesn't support non-integral types");

  decltype(cols_) cs;
  if (!(in >> cs))
    return false;

  decltype(cols_) rs;
  if (!(in >> rs))
    return false;

  decltype(data_) v(cs * rs);

  for (auto &e : v)
    if (!(in >> e))
      return false;

  cols_ = cs;
  data_ = v;

  assert(!empty() || (cols() == 0 && size() == 0));
  return true;
}

///
/// \param[out] o output stream
/// \param[in] m a matrix
///
/// Prints `m` on the output stream. This is mainly used for debug purpose
/// (boost test needs the operator to report errors).
///
template<class T>
std::ostream &operator<<(std::ostream &o, const matrix<T> &m)
{
  std::size_t i(0);

  for (const auto &e : m)
  {
    o << e << (i && (i % m.cols()) == 0 ? '\n' : ' ');

    ++i;
  }

  return o;
}

#endif  // Include guard