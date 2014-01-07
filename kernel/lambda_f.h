/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2012-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(LAMBDA_F_H)
#define      LAMBDA_F_H

#include "kernel/data.h"
#include "kernel/matrix.h"
#include "kernel/src/interpreter.h"

namespace vita
{
  template<class T> class dyn_slot_lambda_f;
  template<class T> class dyn_slot_evaluator;
  template<class T> class gaussian_lambda_f;
  template<class T> class gaussian_evaluator;

  ///
  /// \brief Transforms individuals into lambda functions
  ///
  /// \tparam T type of individual.
  ///
  /// This class transforms individuals into lambda functions which can be
  /// used to calculate the answers for symbolic regression /
  /// classification problems.
  ///
  /// \note
  /// Depending on the task (regression, classification...) lambda_f and
  /// interpreter outputs can be similar or distinct.
  /// E.g. for *regression problems* lambda_f and interpreter are identical:
  /// they calculate the same number.
  /// lambda_f always calculates a meaningful value for the end-user (the
  /// class of the example, an approximation...) while interpreter can output
  /// a value that is just a building block for lambda_f (e.g. classification
  /// tasks with discriminant functions).
  /// The typical use chain is: evaluator uses lambda_f, lambda_f uses
  /// interpreter.
  ///
  /// Another interesting function of lambda_f is that it extends the
  /// functionalities of interpreter to teams.
  ///
  template<class T>
  class lambda_f
  {
  public:
    explicit lambda_f(const T &);

    virtual any operator()(const data::example &) const = 0;

    virtual std::string name(const any &) const;

  protected:
    T prg_;
  };

  ///
  /// \brief Transforms individual to a lambda function for regression
  ///
  /// \tparam T type of individual.
  ///
  template<class T>
  class reg_lambda_f : public lambda_f<T>
  {
  public:
    explicit reg_lambda_f(const T &);

    virtual any operator()(const data::example &) const final;

  protected:
    mutable src_interpreter<T> int_;
  };

  template<>
  template<class T>
  class reg_lambda_f<team<T>> : public lambda_f<team<T>>
  {
  public:
    explicit reg_lambda_f(const team<T> &);

    virtual any operator()(const data::example &) const final;

  protected:
    mutable std::vector<src_interpreter<T>> int_;
  };

  ///
  /// \tparam T type of individual.
  ///
  /// This class is used to factorize out some code from the lambda functions
  /// used for classification tasks.
  ///
  template<class T>
  class class_lambda_f : public lambda_f<T>
  {
  public:
    class_lambda_f(const T &, const data &);

    virtual std::string name(const any &) const final;

  protected:
    /// class_name_[i] = "name of the i-th class of the classification task".
    std::vector<std::string> class_name_;
  };

  ///
  /// \tparam T type of individual.
  ///
  /// This class encapsulates the engine of the Slotted Dynamic Class
  /// Boundary Determination algorithm (see vita::dyn_slot_evaluator for
  /// further details about the algorithm).
  ///
  /// Methods and properties of dyn_slot_engine can only be accessed from
  /// vita::dyn_slot_lambda_f and dyn_slot_evaluator (they have a HAS-A
  /// 'friendly' relationship with dyn_slot_engine).
  ///
  template<class T>
  class dyn_slot_engine
  {
  protected:
    friend class dyn_slot_lambda_f<T>;
    friend class dyn_slot_evaluator<T>;

    dyn_slot_engine() {}
    dyn_slot_engine(const T &, data &, unsigned);

    unsigned slot(const T &, const data::example &) const;

    /// The main matrix of the dynamic slot algorithm.
    /// slot_matrix[slot][class] = "number of training examples of class
    /// 'class' mapped to slot 'slot'".
    matrix<unsigned> slot_matrix;

    /// slot_class[i] = "label of the predominant class" for the i-th slot.
    std::vector<unsigned> slot_class;

    /// Size of the dataset used to construct \a slot_matrix.
    unsigned dataset_size;

    double training_accuracy() const;

    static number normalize_01(number);

  private:  // Private support method
    template<class U> void fill_matrix(const U &, data &);
    template<class U> void fill_matrix(const team<U> &, data &);
  };

  ///
  /// \tparam T type of individual.
  ///
  /// This class transforms individuals to lambda functions which can be used
  /// for classification tasks.
  ///
  /// The algorithm used for classification is Slotted Dynamic Class
  /// Boundary Determination (see vita::dyn_slot_evaluator for further details).
  ///
  template<class T>
  class dyn_slot_lambda_f : public class_lambda_f<T>
  {
  public:
    dyn_slot_lambda_f(const T &, data &, unsigned);

    virtual any operator()(const data::example &) const override;

  private:
    dyn_slot_engine<T> engine_;
  };

  ///
  /// \tparam T type of individual.
  ///
  /// This class encapsulates the engine of the Gaussian classification
  /// algorithm (see vita::gaussian_evaluator for further details).
  ///
  /// Methods and properties of gaussian_engine can only be accessed from
  /// vita::gaussian_lambda_f and gaussian_evaluator (they have a HAS-A
  /// 'friendly' relationship with gaussian_engine).
  ///
  template<class T>
  class gaussian_engine
  {
    friend class gaussian_lambda_f<T>;
    friend class gaussian_evaluator<T>;

    gaussian_engine() {}
    gaussian_engine(const T &, data &);

    unsigned class_label(const T &, const data::example &,
                         number * = nullptr, number * = nullptr) const;

    /// gauss_dist[i] = "the gaussian distribution of the i-th class if the
    /// classification problem".
    std::vector<distribution<number>> gauss_dist;
  };

  ///
  /// \tparam T type of individual.
  ///
  /// This class transforms individuals to lambda functions which can be used
  /// for classification tasks.
  ///
  /// The algorithm used for classification is Slotted Dynamic Class
  /// Boundary Determination (see vita::dyn_slot_evaluator for further details).
  ///
  template<class T>
  class gaussian_lambda_f : public class_lambda_f<T>
  {
  public:
    gaussian_lambda_f(const T &, data &);

    virtual any operator()(const data::example &) const override;

  private:
    gaussian_engine<T> engine_;
  };

  ///
  /// \tparam T type of individual.
  ///
  /// This class transforms individuals to lambda functions which can be used
  /// for single-class classification tasks.
  ///
  template<class T>
  class binary_lambda_f : public class_lambda_f<T>
  {
  public:
    binary_lambda_f(const T &, data &);

    virtual any operator()(const data::example &) const override;
  };

#include "kernel/lambda_f_inl.h"
}  // namespace vita

#endif  // LAMBDA_F_H
