// Copyright 2020 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.
#include "pw_i2c/initiator_mock.h"

#include <iostream>

#include "pw_assert/check.h"

namespace pw::i2c {

Status MockInitiator::DoWriteReadFor(
    Address device_address,
    ConstByteSpan tx_buffer,
    ByteSpan rx_buffer,
    chrono::SystemClock::duration for_at_least) {
  PW_CHECK_INT_LT(expected_transaction_index_, expected_transactions_.size());

  EXPECT_EQ(
      expected_transactions_[expected_transaction_index_].address().GetTenBit(),
      device_address.GetTenBit());

  auto expected_for_at_least =
      expected_transactions_[expected_transaction_index_].for_at_least();
  if (expected_for_at_least.has_value()) {
    EXPECT_EQ(expected_for_at_least.value(), for_at_least);
  }

  ConstByteSpan expected_tx_buffer =
      expected_transactions_[expected_transaction_index_].write_buffer();
  EXPECT_TRUE(std::equal(expected_tx_buffer.begin(),
                         expected_tx_buffer.end(),
                         tx_buffer.begin(),
                         tx_buffer.end()));

  ConstByteSpan expected_rx_buffer =
      expected_transactions_[expected_transaction_index_].read_buffer();
  EXPECT_EQ(expected_rx_buffer.size(), rx_buffer.size());

  std::copy(
      expected_rx_buffer.begin(), expected_rx_buffer.end(), rx_buffer.begin());

  // Do not directly return this value as expected_transaction_index_ should be
  // incremented.
  const Status expected_return_value =
      expected_transactions_[expected_transaction_index_].return_value();

  expected_transaction_index_ += 1;

  return expected_return_value;
}

MockInitiator::~MockInitiator() { EXPECT_EQ(Finalize(), OkStatus()); }

}  // namespace pw::i2c