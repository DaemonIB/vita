/**
 *
 *  \file example7.cc
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  Building blocks infrastructure test.
 */

#include <cstdlib>
#include <iostream>
#include <fstream>

#include "kernel/adf.h"
#include "kernel/distribution.h"
#include "kernel/environment.h"
#include "kernel/primitive/factory.h"

int main(int argc, char *argv[])
{
  vita::environment env;

  env.code_length = argc > 1 ? atoi(argv[1]) : 100;
  const unsigned n(argc > 2 ? atoi(argv[2]) : 1);

  vita::symbol_factory &factory(vita::symbol_factory::instance());
  env.insert(factory.make("NUMBER", vita::d_double, -200, 200));
  env.insert(factory.make("ADD", vita::d_double));
  env.insert(factory.make("SUB", vita::d_double));
  env.insert(factory.make("MUL", vita::d_double));
  env.insert(factory.make("IFL", vita::d_double));
  env.insert(factory.make("IFE", vita::d_double));
  env.insert(factory.make("ABS", vita::d_double));
  env.insert(factory.make("LN", vita::d_double));

  vita::distribution<double> individuals, blocks_len, blocks_n, arguments;

  for (unsigned k(0); k < n; ++k)
  {
    vita::individual base(env, true);
    unsigned base_es(base.eff_size());
    while (base_es < 5)
    {
      base = vita::individual(env, true);
      base_es = base.eff_size();
    }

    individuals.add(base_es);

    std::cout << std::string(40, '-') << std::endl;
    base.list(std::cout);
    std::cout << std::endl;

    std::list<vita::loc_t> bl(base.blocks());
    for (auto i(bl.begin()); i != bl.end(); ++i)
    {
      vita::individual ib(base.get_block(*i));

      std::vector<vita::loc_t> loci;
      vita::individual generalized(ib.generalize(2, &loci));

      std::cout << std::endl;
      ib.list(std::cout);

      std::cout << "GENERALIZED" << std::endl;
      generalized.list(std::cout);

      const unsigned arg_n(loci.size());
      std::cout << std::endl << "Arguments:";
      for (unsigned j(0); j < arg_n; ++j)
        std::cout << " (pos=" << loci[j].index << ",category="
                  << loci[j].category << ")";
        std::cout << std::endl;

        blocks_len.add(ib.eff_size());
        arguments.add(arg_n);
    }
  }

  std::cout << std::string(40, '-') << std::endl
            << "Individuals effective lengths." << std::endl
            << "Min: " << individuals.min << "  Mean: " << individuals.mean
            << "  StdDev: " << std::sqrt(individuals.variance)
            << "  Max: " << individuals.max << std::endl
            << "Blocks effective lengths." << std::endl
            << "Min: " << blocks_len.min << "  Mean: " << blocks_len.mean
            << "  StdDev: " << std::sqrt(blocks_len.variance)
            << "  Max: " << blocks_len.max << std::endl
            << "Number of arguments." << std::endl
            << "Min: " << arguments.min << "  Mean: " << arguments.mean
            << "  StdDev: " << std::sqrt(arguments.variance)
            << "  Max: " << arguments.max << std::endl;

  return EXIT_SUCCESS;
}
