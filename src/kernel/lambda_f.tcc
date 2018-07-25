/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_LAMBDA_F_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_LAMBDA_F_TCC)
#define      VITA_LAMBDA_F_TCC

///
/// \param[in] prg the program (individual/team) to be lambdified.
///
template<class T, bool S>
basic_reg_lambda_f<T, S>::basic_reg_lambda_f(const T &prg)
  : detail::reg_lambda_f_storage<T, S>(prg)
{
  Ensures(debug());
}

///
/// \param[in] e input example for the lambda function.
/// \return the output value associated with `e`.
///
template<class T, bool S>
any basic_reg_lambda_f<T, S>::operator()(const dataframe::example &e) const
{
  // We use tag dispatching by instance (i.e. to delegate to an implementation
  // function that receives standard arguments plus a dummy argument based on a
  // compile-time condition). Usually this is much easier to debug and get
  // right that the std::enable_if solution.
  // Moreover this is almost guaranteed to be optimized away by a decent
  // compiler.
  return eval(e, is_team<T>());
}

template<class T, bool S>
any basic_reg_lambda_f<T, S>::eval(const dataframe::example &e,
                                   std::false_type) const
{
  return this->run(e.input);
}

template<class T, bool S>
any basic_reg_lambda_f<T, S>::eval(const dataframe::example &e,
                                   std::true_type) const
{
  number avg(0), count(0);

  // Calculate the running average.
  for (const auto &core : this->team_)
  {
    const auto res(core.run(e.input));

    if (res.has_value())
      avg += (to<number>(res) - avg) / ++count;
  }

  return count > 0.0 ? any(avg) : any();
}

///
/// \param[in] a a value produced by lambda_f::operator().
/// \return the string version of `a`.
///
template<class T, bool S>
std::string basic_reg_lambda_f<T, S>::name(const any &a) const
{
  return std::to_string(to<number>(a));
}

///
/// \brief Calls (dynamic dispatch) polymhorphic model_metric `m` on `this`
///
/// \param[in] m a metric we are evaluating.
/// \param[in] d a dataset.
/// \return the value of `this` according to metric `m`.
///
template<class T, bool S>
double basic_reg_lambda_f<T, S>::measure(const model_metric<T> &m,
                                         const dataframe &d) const
{
  return m(this, d);
}

///
/// \return `true` if the object passes the internal consistency check.
///
template<class T, bool S>
bool basic_reg_lambda_f<T, S>::debug() const
{
  return detail::reg_lambda_f_storage<T, S>::debug();
}

///
/// \param[in] p active problem.
/// \param[in] in input stream.
/// \return `true` if the lambda has been loaded correctly.
///
/// \note
/// If the load operation isn't successful the current lambda isn't modified.
///
template<class T, bool S>
bool basic_reg_lambda_f<T, S>::load(std::istream &in, const problem &p)
{
  return detail::reg_lambda_f_storage<T, S>::load(in, p);
}

///
/// \param[out] out output stream.
/// \return `true` if lambda was saved correctly.
///
template<class T, bool S>
bool basic_reg_lambda_f<T, S>::save(std::ostream &out) const
{
  return detail::reg_lambda_f_storage<T, S>::save(out);
}

///
/// \param[in] d the training set.
///
template<class T, bool N>
basic_class_lambda_f<T, N>::basic_class_lambda_f(const dataframe &d)
  : detail::class_names<N>(d)
{
}

///
/// \param[in] e example to be classified.
/// \return the label of the class that includes `e` (wrapped in class any).
///
template<class T, bool N>
any basic_class_lambda_f<T, N>::operator()(const dataframe::example &e) const
{
  return any(this->tag(e).first);
}

///
/// \brief Calls (dynamic dispatch) polymhorphic model_metric `m` on `this`
///
/// \param[in] m a metric we are evaluating.
/// \param[in] d a dataset.
/// \return the value of `this` according to metric `m`.
///
template<class T, bool N>
double basic_class_lambda_f<T, N>::measure(const model_metric<T> &m,
                                           const dataframe &d) const
{
  return m(this, d);
}

///
/// \param[in] a id of a class.
/// \return the name of class `a`.
///
template<class T, bool N>
std::string basic_class_lambda_f<T, N>::name(const any &a) const
{
  return detail::class_names<N>::string(a);
}

///
/// \param[in] ind individual "to be transformed" into a lambda function.
/// \param[in] d the training set.
/// \param[in] x_slot number of slots for each class of the training set.
///
template<class T, bool S, bool N>
basic_dyn_slot_lambda_f<T, S, N>::basic_dyn_slot_lambda_f(const T &ind,
                                                          dataframe &d,
                                                          unsigned x_slot)
  : basic_class_lambda_f<T, N>(d), lambda_(ind),
    slot_matrix_(d.classes() * x_slot, d.classes()),
    slot_class_(d.classes() * x_slot), dataset_size_(0)
{
  Expects(ind.debug());
  Expects(d.debug());
  Expects(d.classes() > 1);
  Expects(x_slot);

  fill_matrix(d, x_slot);
}

///
/// \param[in] d the training set.
/// \param[in] x_slot number of slots for each class of the training set.
///
/// Sets up the data structures needed by the 'dynamic slot' algorithm.
///
template<class T, bool S, bool N>
void basic_dyn_slot_lambda_f<T, S, N>::fill_matrix(dataframe &d,
                                                   unsigned x_slot)
{
  Expects(d.debug());
  Expects(d.classes() > 1);
  Expects(x_slot);

  const auto n_slots(d.classes() * x_slot);
  assert(n_slots == slot_matrix_.rows());
  assert(slot_matrix_.cols() == d.classes());

  // Here starts the slot-filling task.
  slot_matrix_.fill(0);

  // In the first step this method evaluates the program to obtain an output
  // value for each training example. Based on the program output a
  // bi-dimensional matrix is built (slot_matrix_(slot, class)).
  for (const auto &example : d)
  {
    ++dataset_size_;

    ++slot_matrix_(slot(example), label(example));
  }

  const auto unknown(d.classes());

  // In the second step the method dynamically determine which class each
  // slot belongs to by simply taking the class with the largest value at the
  // slot...
  for (auto i(decltype(n_slots){0}); i < n_slots; ++i)
  {
    const auto cols(slot_matrix_.cols());
    auto best_class(decltype(cols){0});  // Initially assuming class 0 as best

    // ...then looking for a better class among the remaining ones.
    for (auto j(decltype(cols){1}); j < cols; ++j)
      if (slot_matrix_(i, j) >= slot_matrix_(i, best_class))
        best_class = j;

    slot_class_[i] = slot_matrix_(i, best_class) ? best_class : unknown;
  }

  // Unknown slots can be a problem with new examples (not contained in the
  // training set). We arbitrary assign them to the class of a neighbour
  // slot (if available).
  // Another interesting strategy would be to assign unknown slots to the
  // largest class.
  for (auto i(decltype(n_slots){0}); i < n_slots; ++i)
    if (slot_class_[i] == unknown)
    {
      if (i && slot_class_[i - 1] != unknown)
        slot_class_[i] = slot_class_[i - 1];
      else if (i + 1 < n_slots && slot_class_[i + 1] != unknown)
        slot_class_[i] = slot_class_[i + 1];
      else
        slot_class_[i] = 0;
    }
}

///
/// \param[in] e input data.
/// \return the slot example `e` falls into.
///
template<class T, bool S, bool N>
std::size_t basic_dyn_slot_lambda_f<T,S,N>::slot(
  const dataframe::example &e) const
{
  const any res(lambda_(e));

  const auto ns(slot_matrix_.rows());
  const auto last_slot(ns - 1);
  if (!res.has_value())
    return last_slot;

  const auto val(to<number>(res));
  const auto where(discretization(val, last_slot));

  return (where >= ns) ? last_slot : where;
}

///
/// \return the accuracy of the lambda function on the training set.
///
template<class T, bool S, bool N>
double basic_dyn_slot_lambda_f<T, S, N>::training_accuracy() const
{
  double ok(0.0);

  const auto slots(slot_matrix_.rows());

  for (auto i(decltype(slots){0}); i < slots; ++i)
    ok += slot_matrix_(i, slot_class_[i]);

  assert(dataset_size_ >= ok);

  return ok / dataset_size_;
}

///
/// \param[in] instance data to be classified.
/// \return the class of `instance` (numerical id) and the confidence level
///         (in the range [0,1]).
///
template<class T, bool S, bool N>
std::pair<class_t, double> basic_dyn_slot_lambda_f<T, S, N>::tag(
  const dataframe::example &instance) const
{
  const auto s(slot(instance));
  const auto classes(slot_matrix_.cols());

  unsigned total(0);
  for (auto j(decltype(classes){0}); j < classes; ++j)
    total += slot_matrix_(s, j);

  const auto ok(slot_matrix_(s, slot_class_[s]));

  const double confidence(!total ? 0.5 : static_cast<double>(ok) / total);

  return {slot_class_[s], confidence};
}

///
/// \param[out] out output stream.
/// \return true on success.
///
/// Saves the lambda on persistent storage.
///
template<class T, bool S, bool N>
bool basic_dyn_slot_lambda_f<T, S, N>::save(std::ostream &out) const
{
  if (!lambda_.save(out))
    return false;

  if (!slot_matrix_.save(out))
    return false;

  // Don't need to save slot_class_.size() since it's equal to
  // slot_matrix_.rows()
  for (const auto s : slot_class_)
    out << s << '\n';

  out << dataset_size_ << '\n';

  if (!detail::class_names<N>::save(out))
    return false;

  return out.good();
}

///
/// \brief Loads the lambda from persistent storage
///
/// \param[in] p active problem.
/// \param[in] in input stream.
/// \return true on success.
///
/// \note
/// If the load operation isn't successful the current lambda isn't modified.
///
template<class T, bool S, bool N>
bool basic_dyn_slot_lambda_f<T, S, N>::load(std::istream &in, const problem &p)
{
  // Tag dispatching to select the appropriate method.
  // Note that there is an implementation of operator= only for S==true.
  // Without tag dispatching the compiler would need a complete implementation
  // (but we haven't a reasonable/general solution for the S==false case).
  return load_(in, p, detail::is_true<S>());
}

///
/// \param[in] p current problem.
/// \param[in] in input stream.
/// \return `true` on success.
///
/// This is part of the tag dispatching method used by the
/// basic_dyn_slot_lambda_f::load method.
///
template<class T, bool S, bool N>
bool basic_dyn_slot_lambda_f<T, S, N>::load_(std::istream &in,
                                             const problem &p, std::true_type)
{
  decltype(lambda_) l(lambda_);
  if (!l.load(in, p))
    return false;

  decltype(slot_matrix_) m;
  if (!m.load(in))
    return false;

  decltype(slot_class_) s(slot_matrix_.rows());
  for (auto &r : s)
    if (!(in >> r))
      return false;

  decltype(dataset_size_) d;
  if (!(in >> d))
    return false;

  if (!detail::class_names<N>::load(in))
    return false;

  lambda_ = l;
  slot_matrix_ = m;
  slot_class_ = s;
  dataset_size_ = d;

  return true;
}

template<class T, bool S, bool N>
bool basic_dyn_slot_lambda_f<T, S, N>::load_(std::istream &, const problem &,
                                             std::false_type)
{
  return false;
}

///
/// \return `true` if the object passes the internal consistency check.
///
template<class T, bool S, bool N>
bool basic_dyn_slot_lambda_f<T, S, N>::debug() const
{
  if (slot_matrix_.cols() <= 1)  // Too few classes
    return false;

  if (slot_matrix_.rows() != slot_class_.size())
    return false;

  return lambda_.debug();
}

///
/// \param[in] ind individual "to be transformed" into a lambda function.
/// \param[in] d the training set.
///
template<class T, bool S, bool N>
basic_gaussian_lambda_f<T, S, N>::basic_gaussian_lambda_f(const T &ind,
                                                          dataframe &d)
  : basic_class_lambda_f<T, N>(d), lambda_(ind), gauss_dist_(d.classes())
{
  Expects(ind.debug());
  Expects(d.debug());
  Expects(d.classes() > 1);

  fill_vector(d);
}

///
/// \param[in] d the training set.
///
/// Sets up the data structures needed by the gaussian algorithm.
///
template<class T, bool S, bool N>
void basic_gaussian_lambda_f<T, S, N>::fill_vector(dataframe &d)
{
  Expects(d.classes() > 1);

  // For a set of training data, we assume that the behaviour of a program
  // classifier is modelled using multiple Gaussian distributions, each of
  // which corresponds to a particular class. The distribution of a class is
  // determined by evaluating the program on the examples of the class in
  // the training set. This is done by taking the mean and standard deviation
  // of the program outputs for those training examples for that class.
  for (const auto &example : d)
  {
    const any res(lambda_(example));

    number val(res.has_value() ? to<number>(res) : 0.0);
    const number cut(10000000.0);
    if (val > cut)
      val = cut;
    else if (val < -cut)
      val = -cut;

    gauss_dist_[label(example)].add(val);
  }
}

///
/// \param[in] example input value whose class we are interested in.
/// \return the class of `example` (numerical id) and the confidence level
///         (how sure you can be that `example` is properly classified. The
///         value is in [0;1] range and the sum of all the confidence levels of
///         each class is 1).
///
template<class T, bool S, bool N>
std::pair<class_t, double> basic_gaussian_lambda_f<T, S, N>::tag(
  const dataframe::example &example) const
{
  const any res(lambda_(example));
  const number x(res.has_value() ? to<number>(res) : 0.0);

  number val_(0.0), sum_(0.0);
  class_t probable_class(0);

  const auto classes(static_cast<unsigned>(gauss_dist_.size()));
  for (auto i(decltype(classes){0}); i < classes; ++i)
  {
    const number distance(std::fabs(x - gauss_dist_[i].mean()));
    const number variance(gauss_dist_[i].variance());

    number p(0.0);
    if (issmall(variance))     // These are borderline cases
      if (issmall(distance))   // These are borderline cases
        p = 1.0;
      else
        p = 0.0;
    else                       // This is the standard case
      p = std::exp(-distance * distance / variance);

    if (p > val_)
    {
      val_ = p;
      probable_class = i;
    }

    sum_ += p;
  }

  // Normalized confidence value.
  // Do not change sum_ > 0.0 with
  // - issmall(sum_) => when sum_ is small, val_ is smaller and the division
  //                    works well.
  // - sum_ => it's the same thing but will produce a warning with
  //           -Wfloat-equal
  const double confidence(sum_ > 0.0 ? val_ / sum_ : 0.0);

  return {probable_class, confidence};
}

///
/// \param[out] out output stream.
/// \return true on success.
///
/// Saves the lambda on persistent storage.
///
template<class T, bool S, bool N>
bool basic_gaussian_lambda_f<T, S, N>::save(std::ostream &out) const
{
  if (!lambda_.save(out))
    return false;

  out << gauss_dist_.size() << '\n';
  for (const auto g : gauss_dist_)
    if (!g.save(out))
      return false;

  if (!detail::class_names<N>::save(out))
    return false;

  return out.good();
}

///
/// \param[in] p current problem.
/// \param[in] in input stream.
/// \return true on success.
///
/// Loads the lambda from persistent storage.
///
/// \note
/// If the load operation isn't successful the current lambda isn't modified.
///
template<class T, bool S, bool N>
bool basic_gaussian_lambda_f<T, S, N>::load(std::istream &in, const problem &p)
{
  // Tag dispatching to select the appropriate method.
  // Note that there is an implementation of operator= only for S==true.
  // Without tag dispatching the compiler would need a complete implementation
  // (but we haven't a reasonable/general solution for the S==false case).
  return load_(in, p, detail::is_true<S>());
}

///
/// \param[in] p current problem.
/// \param[in] in input stream.
/// \return `true` on success.
///
/// This is part of the tag dispatching method used by the
/// basic_gaussian_lambda_f::load method.
///
template<class T, bool S, bool N>
bool basic_gaussian_lambda_f<T, S, N>::load_(std::istream &in,
                                             const problem &p, std::true_type)
{
  decltype(lambda_) l(lambda_);
  if (!l.load(in, p))
    return false;

  typename decltype(gauss_dist_)::size_type n;
  if (!(in >> n))
    return false;

  decltype(gauss_dist_) g(n);
  for (auto &d : g)
    if (!d.load(in))
      return false;

  if (!detail::class_names<N>::load(in))
    return false;

  lambda_ = l;
  gauss_dist_ = g;

  return true;
}

///
/// This is part of the tag dispatching method used by the
/// basic_gaussian_lambda_f::load method.
///
template<class T, bool S, bool N>
bool basic_gaussian_lambda_f<T, S, N>::load_(std::istream &, const problem &,
                                             std::false_type)
{
  return false;
}

///
/// \return `true` if the object passes the internal consistency check.
///
template<class T, bool S, bool N>
bool basic_gaussian_lambda_f<T, S, N>::debug() const
{
  return lambda_.debug();
}

///
/// \param[in] ind individual "to be transformed" into a lambda function.
/// \param[in] d the training set.
///
template<class T, bool S, bool N>
basic_binary_lambda_f<T, S, N>::basic_binary_lambda_f(const T &ind,
                                                      dataframe &d)
  : basic_class_lambda_f<T, N>(d), lambda_(ind)
{
  Expects(ind.debug());
  Expects(d.debug());
  Expects(d.classes() == 2);
}

///
/// \param[in] e input example for the lambda function.
/// \return the class of `e` (numerical id) and the confidence level (in the
///         range [0,1]).
///
template<class T, bool S, bool N>
std::pair<class_t, double> basic_binary_lambda_f<T, S, N>::tag(
  const dataframe::example &e) const
{
  const any res(lambda_(e));
  const number val(res.has_value() ? to<number>(res) : 0.0);

  return {val > 0.0 ? 1u : 0u, std::fabs(val)};
}

///
/// \return `true` if the object passes the internal consistency check.
///
template<class T, bool S, bool N>
bool basic_binary_lambda_f<T, S, N>::debug() const
{
  return lambda_.debug();
}

///
/// \param[out] out output stream.
/// \return true on success.
///
/// Saves the lambda on persistent storage.
///
template<class T, bool S, bool N>
bool basic_binary_lambda_f<T, S, N>::save(std::ostream &out) const
{
  if (!lambda_.save(out))
    return false;

  if (!detail::class_names<N>::save(out))
    return false;

  return out.good();
}

///
/// \param[in] p current problem.
/// \param[in] in input stream.
/// \return true on success.
///
/// Loads the lambda from persistent storage.
///
/// \note
/// If the load operation isn't successful the current lambda isn't modified.
///
template<class T, bool S, bool N>
bool basic_binary_lambda_f<T, S, N>::load(std::istream &in, const problem &p)
{
  // Tag dispatching to select the appropriate method.
  // Note that there is an implementation of operator= only for S==true.
  // Without tag dispatching the compiler would need a complete implementation
  // (but we haven't a reasonable/general solution for the S==false case).
  return load_(in, p, detail::is_true<S>());
}

///
/// \param[in] p current problem.
/// \param[in] in input stream.
/// \return `true` on success.
///
/// This is part of the tag dispatching method used by the
/// basic_binary_lambda_f::load method.
///
template<class T, bool S, bool N>
bool basic_binary_lambda_f<T, S, N>::load_(std::istream &in, const problem &p,
                                           std::true_type)
{
  decltype(lambda_) l(lambda_);
  if (!l.load(in, p))
    return false;

  if (!detail::class_names<N>::load(in))
    return false;

  lambda_ = l;

  return true;
}

///
/// This is part of the tag dispatching method used by the
/// basic_binary_lambda_f::load method.
///
template<class T, bool S, bool N>
bool basic_binary_lambda_f<T, S, N>::load_(std::istream &, const problem &,
                                           std::false_type)
{
  return false;
}

///
/// \param[in] t team "to be transformed" into a lambda function.
/// \param[in] d the training set.
/// \param[in] args auxiliary parameters for the specific lambda function.
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
template<class... Args>
team_class_lambda_f<T, S, N, L, C>::team_class_lambda_f(const team<T> &t,
                                                        dataframe &d,
                                                        Args&&... args)
  : basic_class_lambda_f<team<T>, N>(d), classes_(d.classes())
{
  team_.reserve(t.individuals());
  for (const auto &ind : t)
    team_.emplace_back(ind, d, std::forward<Args>(args)...);
}

///
/// \param[in] instance data to be classified.
/// \return the class of `instance` (numerical id) and the confidence level
///         (in the range [0,1]).
///
/// Specialized method for teams.
///
/// * `team_composition::mv` the class which most of the individuals predict
///   for a given example is selected as team output.
/// * `team_composition::wta` the winner is the individual with the highest
///   confidence in its decision. Specialization may emerge if different
///   members of the team win this contest for different fitness cases (of
///   curse, it is not a feasible alternative to select the member with the
///   best fitness. Then a decision on unknown data is only possible if the
///   right outputs are known in advance and is not made by the team itself).
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
std::pair<class_t, double> team_class_lambda_f<T, S, N, L, C>::tag(
  const dataframe::example &instance) const
{
  if (C == team_composition::wta)
  {
    const auto size(team_.size());
    auto best(team_[0].tag(instance));

    for (auto i(decltype(size){1}); i < size; ++i)
    {
      const auto res(team_[i].tag(instance));

      if (res.second > best.second)
        best = res;
    }

    return best;
  }
  else if (C == team_composition::mv)
  {
    std::vector<unsigned> votes(classes_);

    for (const auto &lambda : team_)
      ++votes[lambda.tag(instance).first];

    class_t max(0);
    for (auto i(max + 1); i < classes_; ++i)
      if (votes[i] > votes[max])
        max = i;

    return {max, static_cast<double>(votes[max]) /
            static_cast<double>(team_.size())};
  }
}

///
/// \brief Saves the lambda team on persistent storage
///
/// \param[out] out output stream.
/// \return true on success.
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
bool team_class_lambda_f<T, S, N, L, C>::save(std::ostream &out) const
{
  out << classes_ << '\n';

  out << team_.size() << '\n';
  for (const auto &i : team_)
    if (!i.save(out))
      return false;

  if (!detail::class_names<N>::save(out))
    return false;

  return out.good();
}

///
/// \brief Loads the lambda team from persistent storage
///
/// \param[in] p current problem.
/// \param[in] in input stream.
/// \return `true` on success.
///
/// \note
/// If the load operation isn't successful the current lambda isn't modified.
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
bool team_class_lambda_f<T, S, N, L, C>::load(std::istream &in,
                                              const problem &p)
{
  decltype(classes_) cl;
  if (!(in >> cl))
    return false;

  typename decltype(team_)::size_type s;
  if (!(in >> s))
    return false;

  decltype(team_) t;
  for (unsigned i(0); i < s; ++i)
  {
    typename decltype(team_)::value_type lambda(team_[0]);
    if (!lambda.load(in, p))
      return false;
    t.push_back(lambda);
  }

  if (!detail::class_names<N>::load(in))
    return false;

  team_ = t;
  classes_ = cl;

  return true;
}

///
/// \return `true` if the object passes the internal consistency check.
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
bool team_class_lambda_f<T, S, N, L, C>::debug() const
{
  for (const auto &l : team_)
    if (!l.debug())
      return false;

  return classes_ > 1;
}

#endif  // Include guard
