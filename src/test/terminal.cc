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

#include <sstream>

#include "kernel/src/constant.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("TERMINAL")
{

TEST_CASE("Base")
{
  vita::constant<std::string> t("A TERMINAL");

  CHECK(!t.arity());
  CHECK(!t.auto_defined());
  CHECK(t.debug());
}

}  // TEST_SUITE("TERMINAL")
