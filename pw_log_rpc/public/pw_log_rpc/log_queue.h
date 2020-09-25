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

#pragma once

#include "pw_bytes/span.h"
#include "pw_result/result.h"
#include "pw_ring_buffer/prefixed_entry_ring_buffer.h"
#include "pw_status/status.h"
#include "pw_status/status_with_size.h"

// LogQueue is a ring-buffer queue of log messages. LogQueue is backed by
// a caller-provided byte array and stores its messages in the format
// dictated by the pw_rpc_log log.proto format.
//
// Logs can be returned as a repeated proto message and the output of this
// class can be directly fed into an RPC stream.
//
// Push logs:
// 0) Create LogQueue instance.
// 1) LogQueue::PushTokenizedMessage().
//
// Pop logs:
// 0) Use exsiting LogQueue instance.
// 1) For single entires, LogQueue::Pop().
// 2) For multiple entries, LogQueue::PopMultiple().
namespace pw::log_rpc {

class LogQueue {
 public:
  LogQueue(ByteSpan log_buffer, ByteSpan encode_buffer)
      : encode_buffer_(encode_buffer), ring_buffer_(true) {
    ring_buffer_.SetBuffer(log_buffer);
  }

  LogQueue(const LogQueue&) = delete;
  LogQueue& operator=(const LogQueue&) = delete;
  LogQueue(LogQueue&&) = delete;
  LogQueue& operator=(LogQueue&&) = delete;

  // Construct a LogEntry proto message and push it into the ring buffer.
  // Returns:
  //
  //  OK - success.
  //  FAILED_PRECONDITION - Failed when encoding the proto message.
  //  RESOURCE_EXHAUSTED - Not enough space in the buffer to write the entry.
  pw::Status PushTokenizedMessage(ConstByteSpan message,
                                  uint32_t flags,
                                  uint32_t level,
                                  uint32_t line,
                                  uint32_t thread,
                                  int64_t timestamp);

  // Pop the oldest LogEntry from the queue into the provided buffer.
  // On success, the size is the length of the entry, on failure, the size is 0.
  // Returns:
  //
  // For now, don't support batching. This will always use a single absolute
  // timestamp, and not use delta encoding.
  //
  //  OK - success.
  //  OUT_OF_RANGE - No entries in queue to read.
  //  RESOURCE_EXHAUSTED - Destination data std::span was smaller number of
  //  bytes than the data size of the data chunk being read.  Available
  //  destination bytes were filled, remaining bytes of the data chunk were
  //  ignored.
  pw::Result<ConstByteSpan> Pop(ByteSpan entry_buffer);

  // Pop entries from the queue into the provided buffer. The provided buffer is
  // filled until there is insufficient space for the next log entry.
  //
  //  OK - success.
  pw::Result<ConstByteSpan> PopMultiple(ByteSpan entries_buffer);

 private:
  size_t dropped_entries_;
  int64_t latest_dropped_timestamp_;

  ByteSpan encode_buffer_;
  pw::ring_buffer::PrefixedEntryRingBuffer ring_buffer_{true};
};

// LogQueueWithEncodeBuffer is a LogQueue where the internal encode buffer is
// created and managed by this class.
template <size_t kEncodeBufferSize>
class LogQueueWithEncodeBuffer : public LogQueue {
 public:
  LogQueueWithEncodeBuffer(ByteSpan log_buffer)
      : LogQueue(log_buffer, encode_buffer_) {}

 private:
  std::byte encode_buffer_[kEncodeBufferSize];
};

}  // namespace pw::log_rpc
