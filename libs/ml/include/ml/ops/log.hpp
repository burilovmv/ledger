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

#include "math/standard_functions/log.hpp"
#include "ml/ops/ops.hpp"

#include <cassert>

namespace fetch {
namespace ml {
namespace ops {

template <class T>
class Log : public fetch::ml::ops::Ops<T>
{
public:
  using TensorType    = T;
  using DataType      = typename TensorType::Type;
  using SizeType      = typename TensorType::SizeType;
  using VecTensorType = typename Ops<T>::VecTensorType;
  using SPType        = OpLogSaveableParams<T>;

  Log() = default;

  explicit Log(SPType const &sp)
    : Ops<T>(sp)
  {}

  ~Log() override = default;

  std::shared_ptr<OpsSaveableParams> GetOpSaveableParams() override
  {
    auto sp = std::make_shared<SPType>();
    return sp;
  }

  /**
   * elementwise Log
   * @param inputs vector containing one tensor which is the input tensor to Log
   * @return
   */
  void Forward(VecTensorType const &inputs, TensorType &output) override
  {
    assert(inputs.size() == 1);
    assert(output.shape() == this->ComputeOutputShape(inputs));

    fetch::math::Log((*inputs.at(0)), output);
  }

  /**
   * elementwise log gradient is 1/x * error:
   * f'(input0)= error_signal/input0
   */
  std::vector<TensorType> Backward(VecTensorType const &inputs,
                                   TensorType const &   error_signal) override
  {
    assert(inputs.size() == 1);
    assert(error_signal.shape() == this->ComputeOutputShape(inputs));

    TensorType ret_error_signal(inputs.at(0)->shape());
    fetch::math::Divide(error_signal, (*inputs.at(0)), ret_error_signal);

    return {ret_error_signal};
  }

  std::vector<SizeType> ComputeOutputShape(VecTensorType const &inputs) const override
  {
    return inputs.front()->shape();
  }

  static constexpr OpType OpCode()
  {
    return OpType::OP_LOG;
  }
  static constexpr char const *DESCRIPTOR = "Log";
};

}  // namespace ops
}  // namespace ml
}  // namespace fetch
