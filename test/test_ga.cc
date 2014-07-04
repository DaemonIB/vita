/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/ga/i_num_ga.h"
#include "kernel/ga/evaluator.h"
#include "kernel/ga/interpreter.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE t_ga
#include <boost/test/unit_test.hpp>

using namespace boost;

constexpr double epsilon(0.00001);

#include "factory_fixture5.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_ga, F_FACTORY5)

BOOST_AUTO_TEST_CASE(Interpreter)
{
  vita::i_num_ga ind(env, sset);
  BOOST_REQUIRE(ind.debug());

  BOOST_TEST_CHECKPOINT("First function");
  auto f = [](const std::vector<double> &v)
           { return std::accumulate(v.begin(), v.end(), 0.0); };

  vita::interpreter<vita::i_num_ga>::function = f;
  vita::interpreter<vita::i_num_ga> intr(ind);
  BOOST_REQUIRE(intr.debug());

  ind = {0.0, 0.0, 0.0, 0.0} ;
  vita::any ret(intr.run());
  BOOST_REQUIRE_SMALL(vita::to<vita::ga::base_t>(ret), epsilon);

  ind = {1.0, 1.0, 1.0, 1.0};
  ret = intr.run();
  BOOST_REQUIRE_CLOSE(vita::to<vita::ga::base_t>(ret), 4.0, epsilon);

  ind = {1.0, -1.0, 1.0, -1.0};
  ret = intr.run();
  BOOST_REQUIRE_SMALL(vita::to<vita::ga::base_t>(ret), epsilon);

  ind = {-1.0, -2.0, -3.0, -4.0};
  ret = intr.run();
  BOOST_REQUIRE_CLOSE(vita::to<vita::ga::base_t>(ret), -10.0, epsilon);

  BOOST_TEST_CHECKPOINT("Second function");
  auto f2 = [](const std::vector<double> &v) { return v[0] / v[1]; };

  vita::interpreter<vita::i_num_ga>::function = f2;
  vita::interpreter<vita::i_num_ga> intr2(ind);
  BOOST_REQUIRE(intr2.debug());

  ind = {1.0, 0.0, 0.0, 0.0};
  ret = intr2.run();
  BOOST_REQUIRE(ret.empty());

  ind = {-1.0, -2.0, -3.0, -4.0};
  ret = intr2.run();
  BOOST_REQUIRE_CLOSE(vita::to<vita::ga::base_t>(ret), 0.5, epsilon);
}

BOOST_AUTO_TEST_CASE(Evaluator)
{
  auto f = [](const std::vector<double> &v)
           { return std::accumulate(v.begin(), v.end(), 0.0); };
  vita::interpreter<vita::i_num_ga>::function = f;

  vita::any intr_prev;
  vita::fitness_t eva_prev;

  for (unsigned i(0); i < 1000; ++i)
  {
    vita::i_num_ga ind(env, sset);
    BOOST_REQUIRE(ind.debug());

    vita::interpreter<vita::i_num_ga> intr(ind);
    BOOST_REQUIRE(intr.debug());

    vita::evaluator<vita::i_num_ga> eva;
    const vita::fitness_t eva_ret(eva(ind));
    BOOST_REQUIRE_LE(eva_ret, 0.0);

    const vita::any intr_ret(intr.run());
    std::cout << eva_ret << "  " << intr_ret << std::endl;

    if (!intr_prev.empty())
    {
      if (vita::to<vita::ga::base_t>(intr_prev) <
          vita::to<vita::ga::base_t>(intr.run()))
        BOOST_REQUIRE_LT(eva_prev, eva_ret);
      else
        BOOST_REQUIRE_GE(eva_prev, eva_ret);
    }

    intr_prev = intr_ret;
    eva_prev = eva_ret;
  }
}
BOOST_AUTO_TEST_SUITE_END()
