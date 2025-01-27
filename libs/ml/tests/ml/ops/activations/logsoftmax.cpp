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

#include "core/serializers/main_serializer_definition.hpp"
#include "gtest/gtest.h"
#include "math/base_types.hpp"
#include "math/tensor.hpp"
#include "ml/ops/activation.hpp"
#include "ml/serializers/ml_types.hpp"
#include "vectorise/fixed_point/fixed_point.hpp"

template <typename T>
class LogSoftmaxTest : public ::testing::Test
{
};

using MyTypes = ::testing::Types<fetch::math::Tensor<float>, fetch::math::Tensor<double>,
                                 fetch::math::Tensor<fetch::fixed_point::FixedPoint<32, 32>>>;

TYPED_TEST_CASE(LogSoftmaxTest, MyTypes);

TYPED_TEST(LogSoftmaxTest, forward_test)
{
  using DataType   = typename TypeParam::Type;
  using TensorType = TypeParam;

  TensorType data = TensorType::FromString(R"(1, -2, 3, -4, 5, -6, 7, -8)");
  TensorType gt   = TensorType::FromString(
      R"(-6.14520134, -9.14520134, -4.14520134, -11.14520134, -2.14520134, -13.14520134, -0.14520134, -15.14520134)");

  fetch::ml::ops::LogSoftmax<TensorType> op;
  TensorType prediction(op.ComputeOutputShape({std::make_shared<const TensorType>(data)}));
  op.Forward({std::make_shared<const TensorType>(data)}, prediction);

  // test correct values
  ASSERT_TRUE(prediction.AllClose(gt, DataType{1e-3f}, DataType{1e-3f}));
}

TYPED_TEST(LogSoftmaxTest, forward_3d_tensor_axis_0_test)
{
  using DataType   = typename TypeParam::Type;
  using TensorType = TypeParam;
  using SizeType   = typename TypeParam::SizeType;

  TensorType          data({3, 3, 1});
  TensorType          gt({3, 3, 1});
  std::vector<double> data_input({1, -2, 3, -4, 5, -6, 7, -8, 9});
  std::vector<double> gt_input({-2.1328e+00, -5.1328e+00, -1.3285e-01, -9.0001e+00, -1.4008e-04,
                                -1.1000e+01, -2.1269e+00, -1.7127e+01, -1.2693e-01});
  for (SizeType i{0}; i < 3; ++i)
  {
    for (SizeType j{0}; j < 3; ++j)
    {
      data.Set(i, j, 0, static_cast<DataType>(data_input[j + 3 * i]));
      gt.Set(i, j, 0, static_cast<DataType>(gt_input[j + 3 * i]));
    }
  }

  fetch::ml::ops::LogSoftmax<TensorType> op{1};
  TensorType prediction(op.ComputeOutputShape({std::make_shared<const TensorType>(data)}));
  op.Forward({std::make_shared<const TensorType>(data)}, prediction);

  // test correct values
  ASSERT_TRUE(prediction.AllClose(gt, DataType{1e-3f}, DataType{1e-3f}));
}

TYPED_TEST(LogSoftmaxTest, backward_test)
{
  using DataType   = typename TypeParam::Type;
  using TensorType = TypeParam;

  TensorType data  = TensorType::FromString("1; -2; 3; -4; 5; -6; 7; -8");
  TensorType error = TensorType::FromString("0; 0; 0; 1; 1; 1; 0; 0");
  TensorType gt    = TensorType::FromString(
      "-6.4312e-03; -3.2019e-04; -4.7521e-02;  9.9996e-01;  6.4887e-01; 9.9999e-01; -2.59454; "
      "-7.9368e-07");

  fetch::ml::ops::LogSoftmax<TensorType> op{0};
  std::vector<TensorType>                prediction =
      op.Backward({std::make_shared<const TensorType>(data)}, error);

  // test correct values
  ASSERT_TRUE(prediction[0].AllClose(gt, DataType{1e-5f}, DataType{1e-5f}));
}

TYPED_TEST(LogSoftmaxTest, backward_3d_tensor_axis_0_test)
{
  using DataType   = typename TypeParam::Type;
  using TensorType = TypeParam;
  using SizeType   = typename TypeParam::SizeType;

  TensorType          data({3, 3, 1});
  TensorType          error({3, 3, 1});
  TensorType          gt({3, 3, 1});
  std::vector<double> data_input({1, -2, 3, -4, 5, -6, 7, -8, 9});
  std::vector<double> errorInput({0.1, 0, 0, 0, 0.5, 0, 0, 0, 0.9});
  std::vector<double> gt_input({8.8150e-02, -5.8998e-04, -8.7560e-02, -6.1696e-05, 7.0026e-05,
                                -8.3497e-06, -1.0728e-01, -3.2818e-08, 1.0728e-01});
  for (SizeType i{0}; i < 3; ++i)
  {
    for (SizeType j{0}; j < 3; ++j)
    {
      data.Set(i, j, 0, static_cast<DataType>(data_input[j + 3 * i]));
      error.Set(i, j, 0, static_cast<DataType>(errorInput[j + 3 * i]));
      gt.Set(i, j, 0, static_cast<DataType>(gt_input[j + 3 * i]));
    }
  }
  fetch::ml::ops::LogSoftmax<TensorType> op{1};
  std::vector<TensorType>                prediction =
      op.Backward({std::make_shared<const TensorType>(data)}, error);

  // test correct values
  ASSERT_TRUE(prediction[0].AllClose(gt, DataType{1e-5f}, DataType{1e-5f}));
}

TYPED_TEST(LogSoftmaxTest, saveparams_test)
{
  using DataType      = typename TypeParam::Type;
  using TensorType    = TypeParam;
  using VecTensorType = typename fetch::ml::ops::Ops<TensorType>::VecTensorType;
  using SPType        = typename fetch::ml::ops::LogSoftmax<TensorType>::SPType;
  using OpType        = typename fetch::ml::ops::LogSoftmax<TensorType>;

  TensorType data = TensorType::FromString("1, -2, 3, -4, 5, -6, 7, -8");
  TensorType gt   = TensorType::FromString(
      "-6.14520134, -9.14520134, -4.14520134, -11.14520134, -2.14520134, -13.14520134, "
      "-0.14520134, -15.14520134");

  fetch::ml::ops::LogSoftmax<TensorType> op;
  TensorType    prediction(op.ComputeOutputShape({std::make_shared<const TensorType>(data)}));
  VecTensorType vec_data({std::make_shared<const TensorType>(data)});

  op.Forward(vec_data, prediction);
  // extract saveparams
  std::shared_ptr<fetch::ml::OpsSaveableParams> sp = op.GetOpSaveableParams();

  // downcast to correct type
  auto dsp = std::static_pointer_cast<SPType>(sp);

  // serialize
  fetch::serializers::MsgPackSerializer b;
  b << *dsp;

  // deserialize
  b.seek(0);
  auto dsp2 = std::make_shared<SPType>();
  b >> *dsp2;

  // rebuild node
  OpType new_op(*dsp2);

  // check that new predictions match the old
  TensorType new_prediction(op.ComputeOutputShape({std::make_shared<const TensorType>(data)}));
  new_op.Forward(vec_data, new_prediction);

  // test correct values
  EXPECT_TRUE(
      new_prediction.AllClose(prediction, static_cast<DataType>(0), static_cast<DataType>(0)));
}

TYPED_TEST(LogSoftmaxTest, saveparams_backward_3d_tensor_axis_0_test)
{
  using DataType   = typename TypeParam::Type;
  using TensorType = TypeParam;
  using SizeType   = typename TypeParam::SizeType;
  using OpType     = typename fetch::ml::ops::LogSoftmax<TensorType>;
  using SPType     = typename OpType ::SPType;

  TensorType          data({3, 3, 1});
  TensorType          error({3, 3, 1});
  TensorType          gt({3, 3, 1});
  std::vector<double> data_input({1, -2, 3, -4, 5, -6, 7, -8, 9});
  std::vector<double> errorInput({0.1, 0, 0, 0, 0.5, 0, 0, 0, 0.9});
  std::vector<double> gt_input({8.8150e-02, -5.8998e-04, -8.7560e-02, -6.1696e-05, 7.0026e-05,
                                -8.3497e-06, -1.0728e-01, -3.2818e-08, 1.0728e-01});
  for (SizeType i{0}; i < 3; ++i)
  {
    for (SizeType j{0}; j < 3; ++j)
    {
      data.Set(i, j, 0, static_cast<DataType>(data_input[j + 3 * i]));
      error.Set(i, j, 0, static_cast<DataType>(errorInput[j + 3 * i]));
      gt.Set(i, j, 0, static_cast<DataType>(gt_input[j + 3 * i]));
    }
  }
  fetch::ml::ops::LogSoftmax<TensorType> op{1};

  // run op once to make sure caches etc. have been filled. Otherwise the test might be trivial!
  std::vector<TensorType> prediction =
      op.Backward({std::make_shared<const TensorType>(data)}, error);

  // extract saveparams
  std::shared_ptr<fetch::ml::OpsSaveableParams> sp = op.GetOpSaveableParams();

  // downcast to correct type
  auto dsp = std::dynamic_pointer_cast<SPType>(sp);

  // serialize
  fetch::serializers::MsgPackSerializer b;
  b << *dsp;

  // make another prediction with the original op
  prediction = op.Backward({std::make_shared<const TensorType>(data)}, error);

  // deserialize
  b.seek(0);
  auto dsp2 = std::make_shared<SPType>();
  b >> *dsp2;

  // rebuild node
  OpType new_op(*dsp2);

  // check that new predictions match the old
  std::vector<TensorType> new_prediction =
      new_op.Backward({std::make_shared<const TensorType>(data)}, error);

  // test correct values
  EXPECT_TRUE(prediction.at(0).AllClose(
      new_prediction.at(0), fetch::math::function_tolerance<typename TypeParam::Type>(),
      fetch::math::function_tolerance<typename TypeParam::Type>()));
}
