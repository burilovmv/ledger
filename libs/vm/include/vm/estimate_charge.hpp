#pragma once
//------------------------------------------------------------------------------
//
//   Copyright 2018-2019 Fetch.AI Limited
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//------------------------------------------------------------------------------

#include "vm/vm.hpp"

namespace fetch {
namespace vm {

/**
 * @tparam Estimator
 * @tparam Args
 * @param vm
 * @param e charge estimator function. Should take the same parameters as the opcode handler, and
 * return a ChargeAmount
 * @param args
 * @return false if executing the opcode would exceed the specified charge limit; true otherwise.
 */
template <typename... Args>
bool EstimateCharge(VM *const vm, ChargeEstimator<Args...> const &e, Args const &... args)
{
  if (e)
  {
    // compute the estimate for this function invocation
    auto const charge_estimate = e(args...);

    vm->IncreaseChargeTotal(charge_estimate);

    if (vm->GetChargeTotal() > vm->GetChargeLimit())
    {
      vm->RuntimeError("Charge limit exceeded");

      return false;
    }
  }

  return true;
}

}  // namespace vm
}  // namespace fetch
