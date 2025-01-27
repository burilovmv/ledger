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

#include "core/assert.hpp"
#include "math/tensor.hpp"
#include "ml/regularisers/reg_types.hpp"
#include "ml/regularisers/regulariser.hpp"
#include "ml/saveparams/saveable_params.hpp"
#include "ml/state_dict.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace fetch {
namespace ml {
namespace ops {

/*
 * Abstract Ops interface
 */
template <class T>
class Ops
{
public:
  using TensorType    = T;
  using SizeType      = typename TensorType::SizeType;
  using ArrayPtrType  = std::shared_ptr<TensorType>;
  using VecTensorType = std::vector<std::shared_ptr<TensorType const>>;

  virtual ~Ops() = default;

  virtual void                    Forward(VecTensorType const &inputs, TensorType &output) = 0;
  virtual std::vector<TensorType> Backward(VecTensorType const &inputs,
                                           TensorType const &   error_signal)                 = 0;
  /*
   * ComputeOutputShape is usually expensive function and should be used only for initialisation or
   * in ASSERT. On Forward you can use output.shape() and on Backward there is error_signal.shape()
   */
  virtual std::vector<SizeType> ComputeOutputShape(VecTensorType const &inputs) const = 0;

  virtual std::shared_ptr<OpsSaveableParams> GetOpSaveableParams() = 0;

  Ops() = default;

  explicit Ops(OpsSaveableParams const &sp)
  {
    is_training_ = sp.is_training;
  }

  void SetTraining(bool is_training)
  {
    is_training_ = is_training;
  }

  bool IsTraining()
  {
    return is_training_;
  }

protected:
  bool is_training_ = true;
};

/**
 * Provide an interface for any trainable ops
 * @tparam T passes tensors to graph during update step
 */
template <class T>
class Trainable
{
public:
  using TensorType   = T;
  using ArrayPtrType = std::shared_ptr<TensorType>;
  using DataType     = typename TensorType::Type;
  using RegPtrType   = std::shared_ptr<fetch::ml::regularisers::Regulariser<T>>;

  virtual void                           Step(typename T::Type learning_rate)         = 0;
  virtual struct fetch::ml::StateDict<T> StateDict() const                            = 0;
  virtual void              LoadStateDict(struct fetch::ml::StateDict<T> const &dict) = 0;
  virtual TensorType const &get_weights() const                                       = 0;
  virtual TensorType const &get_gradients() const                                     = 0;
  virtual void              ResetGradients()                                          = 0;
  virtual void              ApplyGradient(TensorType const &grad)                     = 0;
  virtual void              ApplyRegularisation()                                     = 0;

  void SetRegularisation(RegPtrType regulariser, DataType regularisation_rate = DataType{0.0})
  {
    regulariser_         = regulariser;
    regularisation_rate_ = regularisation_rate;
  }

protected:
  RegPtrType regulariser_;
  DataType   regularisation_rate_ = static_cast<DataType>(0);
};

}  // namespace ops
}  // namespace ml
}  // namespace fetch
