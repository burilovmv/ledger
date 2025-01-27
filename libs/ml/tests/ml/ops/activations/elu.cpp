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

#include "math/tensor.hpp"

#include "ml/ops/activations/elu.hpp"
#include "ml/serializers/ml_types.hpp"

#include "core/serializers/main_serializer_definition.hpp"
#include "vectorise/fixed_point/fixed_point.hpp"

#include "gtest/gtest.h"
template <typename T>
class EluTest : public ::testing::Test
{
};

using MyTypes = ::testing::Types<fetch::math::Tensor<float>, fetch::math::Tensor<double>,
                                 fetch::math::Tensor<fetch::fixed_point::FixedPoint<32, 32>>>;

TYPED_TEST_CASE(EluTest, MyTypes);

TYPED_TEST(EluTest, forward_test)
{
  using DataType   = typename TypeParam::Type;
  using TensorType = TypeParam;

  TensorType data = TensorType::FromString(R"(1, -2, 3, -4, 5, -6, 7, -8)");
  TensorType gt   = TensorType::FromString(
      R"(1, -1.72932943352677, 3, -1.96336872222253, 5, -1.99504249564667, 7, -1.99932907474419)");

  fetch::ml::ops::Elu<TensorType> op(DataType{2.0});

  TensorType prediction(op.ComputeOutputShape({std::make_shared<const TensorType>(data)}));
  op.Forward({std::make_shared<const TensorType>(data)}, prediction);

  // test correct values
  ASSERT_TRUE(prediction.AllClose(gt, DataType{1e-5f}, DataType{1e-5f}));
}

TYPED_TEST(EluTest, forward_3d_tensor_test)
{
  using DataType   = typename TypeParam::Type;
  using TensorType = TypeParam;
  using SizeType   = typename TypeParam::SizeType;

  TensorType          data({2, 2, 2});
  TensorType          gt({2, 2, 2});
  std::vector<double> data_input({1, -2, 3, -4, 5, -6, 7, -8});
  std::vector<double> gt_input(
      {1, -1.72932943352677, 3, -1.96336872222253, 5, -1.99504249564667, 7, -1.99932907474419});

  for (SizeType i{0}; i < 2; ++i)
  {
    for (SizeType j{0}; j < 2; ++j)
    {
      for (SizeType k{0}; k < 2; ++k)
      {
        data.Set(i, j, k, static_cast<DataType>(data_input[i + 2 * (j + 2 * k)]));
        gt.Set(i, j, k, static_cast<DataType>(gt_input[i + 2 * (j + 2 * k)]));
      }
    }
  }

  fetch::ml::ops::Elu<TensorType> op(DataType{2.0});

  TensorType prediction(op.ComputeOutputShape({std::make_shared<const TensorType>(data)}));
  op.Forward({std::make_shared<const TensorType>(data)}, prediction);

  // test correct values
  ASSERT_TRUE(prediction.AllClose(gt, DataType{1e-5f}, DataType{1e-5f}));
}

TYPED_TEST(EluTest, backward_test)
{
  using DataType   = typename TypeParam::Type;
  using TensorType = TypeParam;

  TensorType data  = TensorType::FromString(R"(1, -2, 3, -4, 5, -6, 7, -8)");
  TensorType error = TensorType::FromString(R"(0, 0, 0, 0.5, 1, 1, 0, 0)");
  TensorType gt    = TensorType::FromString(R"(0, 0, 0, 0.0183156133, 1, 0.0049575567, 0, 0)");

  fetch::ml::ops::Elu<TensorType> op(DataType{2.0});
  std::vector<TensorType>         prediction =
      op.Backward({std::make_shared<const TensorType>(data)}, error);

  // test correct values
  ASSERT_TRUE(prediction[0].AllClose(gt, DataType{1e-5f}, DataType{1e-5f}));
}

TYPED_TEST(EluTest, backward_3d_tensor_test)
{
  using DataType   = typename TypeParam::Type;
  using TensorType = TypeParam;
  using SizeType   = typename TypeParam::SizeType;

  TensorType          data({2, 2, 2});
  TensorType          error({2, 2, 2});
  TensorType          gt({2, 2, 2});
  std::vector<double> data_input({1, -2, 3, -4, 5, -6, 7, -8});
  std::vector<double> errorInput({0, 0, 0, 0.5, 1, 1, 0, 0});
  std::vector<double> gt_input({0, 0, 0, 0.0183156133, 1, 0.0049575567, 0, 0});

  for (SizeType i{0}; i < 2; ++i)
  {
    for (SizeType j{0}; j < 2; ++j)
    {
      for (SizeType k{0}; k < 2; ++k)
      {
        data.Set(i, j, k, static_cast<DataType>(data_input[i + 2 * (j + 2 * k)]));
        error.Set(i, j, k, static_cast<DataType>(errorInput[i + 2 * (j + 2 * k)]));
        gt.Set(i, j, k, static_cast<DataType>(gt_input[i + 2 * (j + 2 * k)]));
      }
    }
  }

  fetch::ml::ops::Elu<TensorType> op(DataType{2.0});
  std::vector<TensorType>         prediction =
      op.Backward({std::make_shared<const TensorType>(data)}, error);

  // test correct values
  ASSERT_TRUE(prediction[0].AllClose(gt, DataType{1e-5f}, DataType{1e-5f}));
}

TYPED_TEST(EluTest, saveparams_test)
{
  using DataType      = typename TypeParam::Type;
  using TensorType    = TypeParam;
  using VecTensorType = typename fetch::ml::ops::Ops<TensorType>::VecTensorType;
  using SPType        = typename fetch::ml::ops::Elu<TensorType>::SPType;

  TensorType data = TensorType::FromString("1, -2, 3, -4, 5, -6, 7, -8");
  TensorType gt   = TensorType::FromString(
      "1, -1.72932943352677, 3, -1.96336872222253, 5, -1.99504249564667, 7, -1.99932907474419");

  fetch::ml::ops::Elu<TensorType> op(DataType{2.0});

  TensorType prediction(op.ComputeOutputShape({std::make_shared<const TensorType>(data)}));
  op.Forward(VecTensorType({std::make_shared<const TensorType>(data)}), prediction);

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
  fetch::ml::ops::Elu<TensorType> new_op(*dsp2);

  // check that new predictions match the old
  TensorType new_prediction(op.ComputeOutputShape({std::make_shared<const TensorType>(data)}));
  new_op.Forward(VecTensorType({std::make_shared<const TensorType>(data)}), new_prediction);

  // test correct values
  EXPECT_TRUE(
      new_prediction.AllClose(prediction, static_cast<DataType>(0), static_cast<DataType>(0)));
}

TYPED_TEST(EluTest, saveparams_backward_3d_tensor_test)
{
  using DataType   = typename TypeParam::Type;
  using TensorType = TypeParam;
  using SizeType   = typename TypeParam::SizeType;
  using OpType     = typename fetch::ml::ops::Elu<TensorType>;
  using SPType     = typename fetch::ml::ops::Elu<TensorType>::SPType;

  TensorType          data({2, 2, 2});
  TensorType          error({2, 2, 2});
  TensorType          gt({2, 2, 2});
  std::vector<double> data_input({1, -2, 3, -4, 5, -6, 7, -8});
  std::vector<double> errorInput({0, 0, 0, 0.5, 1, 1, 0, 0});
  std::vector<double> gt_input({0, 0, 0, 0.0183156133, 1, 0.0049575567, 0, 0});

  for (SizeType i{0}; i < 2; ++i)
  {
    for (SizeType j{0}; j < 2; ++j)
    {
      for (SizeType k{0}; k < 2; ++k)
      {
        data.Set(i, j, k, static_cast<DataType>(data_input[i + 2 * (j + 2 * k)]));
        error.Set(i, j, k, static_cast<DataType>(errorInput[i + 2 * (j + 2 * k)]));
        gt.Set(i, j, k, static_cast<DataType>(gt_input[i + 2 * (j + 2 * k)]));
      }
    }
  }

  fetch::ml::ops::Elu<TensorType> op(DataType{2.0});

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
