// Copyright 2021 The Pigweed Authors
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

#include "gtest/gtest.h"
#include "pw_bytes/array.h"
#include "pw_protobuf/streaming_encoder.h"

namespace pw::protobuf {
namespace {

TEST(Encoder, SizeTypeIsConfigured) {
  static_assert(config::kMaxVarintSize == sizeof(uint8_t));
}

TEST(Encoder, NestedWriteSmallerThanVarintSize) {
  std::array<std::byte, 256> buffer;

  // TODO(pwbug/384): Use Encoder when MemoryEncoder is renamed.
  MemoryEncoder encoder(buffer);

  StreamingEncoder nested = encoder.GetNestedEncoder(1);
  // 1 byte key + 1 byte size + 125 byte value = 127 byte nested length.
  EXPECT_EQ(nested.WriteBytes(2, bytes::Initialized<125>(0xaa)), OkStatus());
  nested.Finalize();

  EXPECT_EQ(encoder.status(), OkStatus());
}

TEST(Encoder, NestedWriteLargerThanVarintSizeReturnsResourceExhausted) {
  std::array<std::byte, 256> buffer;

  // TODO(pwbug/384): Use Encoder when MemoryEncoder is renamed.
  MemoryEncoder encoder(buffer);

  // Try to write a larger nested message than the max nested varint value.
  StreamingEncoder nested = encoder.GetNestedEncoder(1);
  // 1 byte key + 1 byte size + 126 byte value = 128 byte nested length.
  EXPECT_EQ(nested.WriteBytes(2, bytes::Initialized<126>(0xaa)),
            Status::ResourceExhausted());
  EXPECT_EQ(nested.WriteUint32(3, 42), Status::ResourceExhausted());
  nested.Finalize();

  EXPECT_EQ(encoder.status(), Status::ResourceExhausted());
}

TEST(Encoder, NestedMessageLargerThanVarintSizeReturnsResourceExhausted) {
  std::array<std::byte, 256> buffer;

  // TODO(pwbug/384): Use Encoder when MemoryEncoder is renamed.
  MemoryEncoder encoder(buffer);

  // Try to write a larger nested message than the max nested varint value as
  // multiple smaller writes.
  StreamingEncoder nested = encoder.GetNestedEncoder(1);
  EXPECT_EQ(nested.WriteBytes(2, bytes::Initialized<60>(0xaa)), OkStatus());
  EXPECT_EQ(nested.WriteBytes(3, bytes::Initialized<60>(0xaa)), OkStatus());
  EXPECT_EQ(nested.WriteBytes(4, bytes::Initialized<60>(0xaa)),
            Status::ResourceExhausted());
  nested.Finalize();

  EXPECT_EQ(encoder.status(), Status::ResourceExhausted());
}

}  // namespace
}  // namespace pw::protobuf
