/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/environment.h"

namespace vita
{
///
/// \param[in] ss a pointer to the symbol set used in the current environment.
/// \param[in] initialize if `true` initializes every parameter in such a
///                       way as to allow the object to pass
///                       `environment::debug(..., true)`.
///
/// Class constructor. Default values are quite standard, but specific
/// problems need ad-hoc tuning.
///
/// \see search::tune_parameters
///
environment::environment(symbol_set *ss, bool initialize) : sset(ss)
{
  if (initialize)
  {
    code_length = 100;
    patch_length = 1;
    elitism = trilean::yes;
    p_mutation = 0.04;
    p_cross = 0.9;
    brood_recombination = 0;
    dss = trilean::yes;
    layers = 1;
    individuals = 100;
    min_individuals = 2;
    tournament_size = 5;
    mate_zone = 20;
    generations = 100;
    g_without_improvement =
      std::numeric_limits<decltype(g_without_improvement)>::max();
    arl = trilean::no;
    validation_percentage = 20;
  }

  assert(debug(initialize));
}

///
/// \param[out] d output document for saving the environment.
///
/// Saves the environment (XML format).
///
void environment::xml(tinyxml2::XMLDocument *d) const
{
  assert(stat.summary);

  auto *root(d->RootElement());

  auto *e_environment(d->NewElement("environment"));
  root->InsertEndChild(e_environment);
  set_text(e_environment, "layers", layers);
  set_text(e_environment, "individuals", individuals);
  set_text(e_environment, "min_individuals", min_individuals);
  set_text(e_environment, "code_length", code_length);
  set_text(e_environment, "patch_length", patch_length);
  set_text(e_environment, "elitism", as_integer(elitism));
  set_text(e_environment, "mutation_rate", p_mutation);
  set_text(e_environment, "crossover_rate", p_cross);
  set_text(e_environment, "brood_recombination", *brood_recombination);
  set_text(e_environment, "dss", as_integer(dss));
  set_text(e_environment, "tournament_size", tournament_size);
  set_text(e_environment, "mating_zone", *mate_zone);
  set_text(e_environment, "max_generations", generations);
  set_text(e_environment, "max_gens_wo_imp", g_without_improvement);
  set_text(e_environment, "arl", as_integer(arl));
  set_text(e_environment, "validation_percentage", validation_percentage);
  set_text(e_environment, "ttable_bits", ttable_size);  // size 1u<<ttable_size

  auto *e_alps(d->NewElement("alps"));
  e_environment->InsertEndChild(e_alps);
  set_text(e_alps, "age_gap", alps.age_gap);
  set_text(e_alps, "p_same_layer", alps.p_same_layer);

  auto *e_team(d->NewElement("team"));
  e_environment->InsertEndChild(e_team);
  set_text(e_team, "individuals", team.individuals);

  auto *e_statistics(d->NewElement("statistics"));
  e_environment->InsertEndChild(e_statistics);
  set_text(e_statistics, "directory", stat.dir.c_str());
  set_text(e_statistics, "save_arl", stat.arl);
  set_text(e_statistics, "save_dynamics", stat.dynamic);
  set_text(e_statistics, "save_layers", stat.layers);
  set_text(e_statistics, "save_population", stat.population);
  set_text(e_statistics, "save_summary", stat.summary);
}

///
/// \param force_defined all the optional parameter have to be in a
///                      'well defined' state for the function to pass
///                      the test.
/// \return `true` if the object passes the internal consistency check.
///
bool environment::debug(bool force_defined) const
{
  if (force_defined)
  {
    if (!code_length)
    {
      print.error("Undefined code_length data member");
      return false;
    }

    if (!patch_length)
    {
      print.error("Undefined patch_length data member");
      return false;
    }

    if (elitism == trilean::unknown)
    {
      print.error("Undefined elitism data member");
      return false;
    }

    if (p_mutation < 0.0)
    {
      print.error("Undefined p_mutation data member");
      return false;
    }

    if (p_cross < 0.0)
    {
      print.error("Undefined p_cross data member");
      return false;
    }

    if (!brood_recombination)
    {
      print.error("Undefined brood_recombination data member");
      return false;
    }

    if (dss == trilean::unknown)
    {
      print.error("Undefined dss data member");
      return false;
    }

    if (!layers)
    {
      print.error("Undefined layer data member");
      return false;
    }

    if (!individuals)
    {
      print.error("Undefined `individuals` data member");
      return false;
    }

    if (!min_individuals)
    {
      print.error("Undefined `min_individuals` data member");
      return false;
    }

    if (!tournament_size)
    {
      print.error("Undefined tournament_size data member");
      return false;
    }

    if (!mate_zone)
    {
      print.error("Undefined mate_zone data member");
      return false;
    }

    if (!generations)
    {
      print.error("Undefined generations data member");
      return false;
    }

    if (!g_without_improvement)
    {
      print.error("Undefined g_without_improvement data member");
      return false;
    }

    if (arl == trilean::unknown)
    {
      print.error("Undefined arl data member");
      return false;
    }

    if (validation_percentage > 100)
    {
      print.error("Undefined validation_percentage data member");
      return false;
    }

    if (!alps.age_gap)
    {
      print.error("Undefined age_gap parameter");
      return false;
    }

    if (alps.p_same_layer < 0.0)
    {
      print.error("Undefined p_same_layer parameter");
      return false;
    }

    if (!team.individuals)
    {
      print.error("Undefined team size parameter");
      return false;
    }
  }  // if (force_defined)

  if (code_length == 1)
  {
    print.error("code_length is too short");
    return false;
  }

  if (code_length && patch_length && patch_length >= code_length)
  {
    print.error("patch_length must be shorter than code_length");
    return false;
  }

  if (p_mutation > 1.0)
  {
    print.error("p_mutation out of range");
    return false;
  }

  if (p_cross > 1.0)
  {
    print.error("p_cross out of range");
    return false;
  }

  if (alps.p_same_layer > 1.0)
  {
    print.error("`p_same_layer` out of range");
    return false;
  }

  if (min_individuals == 1)
  {
    print.error("At least 2 individuals for layer");
    return false;
  }

  if (individuals && min_individuals && individuals < min_individuals)
  {
    print.warning("Too few individuals");
    return false;
  }

  if (individuals && tournament_size && tournament_size > individuals)
  {
    print.error("tournament_size (", tournament_size,
                ") cannot be greater than population size (", individuals,
                ")");
    return false;
  }

  if (mate_zone && tournament_size && tournament_size > *mate_zone)
  {
    print.error("tournament_size (", tournament_size,
                ") cannot be greater than mate_zone (", *mate_zone, ")");
    return false;
  }

  return true;
}
}  // namespace vita
