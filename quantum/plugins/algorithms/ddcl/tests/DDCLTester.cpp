/*******************************************************************************
 * Copyright (c) 2019 UT-Battelle, LLC.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompanies this
 * distribution. The Eclipse Public License is available at
 * http://www.eclipse.org/legal/epl-v10.html and the Eclipse Distribution
 *License is available at https://eclipse.org/org/documents/edl-v10.php
 *
 * Contributors:
 *   Alexander J. McCaskey - initial API and implementation
 *******************************************************************************/
#include <gtest/gtest.h>

#include "xacc.hpp"
#include "xacc_service.hpp"
#include "Optimizer.hpp"
#include "Algorithm.hpp"

using namespace xacc;
const std::string src = R"rucc(__qpu__ void f(qbit q, double t0, double t1, double t2) {
    Rx(q[0], t0);
    Ry(q[0], t1);
    Rx(q[0], t2);
})rucc";

TEST(VQETester, checkSimpleGradientFree) {
  if (xacc::hasAccelerator("local-ibm")) {
    auto acc = xacc::getAccelerator("local-ibm");
    auto buffer = xacc::qalloc(1);

    auto compiler = xacc::getCompiler("xasm");

    auto ir = compiler->compile(src, acc);
    auto simple = ir->getComposite("f");

    // get cobyla optimizer
    auto optimizer = xacc::getOptimizer("nlopt", HeterogeneousMap{std::make_pair("nlopt-maxeval", 20)});

    std::vector<double> target {.5, .5};

    auto ddcl = xacc::getService<Algorithm>("ddcl");
    EXPECT_TRUE(ddcl->initialize({std::make_pair("ansatz",simple),
                                std::make_pair("accelerator",acc),
                                std::make_pair("target_dist", target),
                                std::make_pair("loss", "js"),
                                std::make_pair("optimizer",optimizer)}));
    ddcl->execute(buffer);

    std::cout << buffer->getInformation("opt-params").as<std::vector<double>>() << "\n";
  }
}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}