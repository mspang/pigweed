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

#include "pw_assert_log/assert_log.h"

#include "pw_assert/options.h"

extern "C" void pw_assert_HandleFailure(void) {
#if PW_ASSERT_ENABLE_DEBUG
  PW_LOG(PW_LOG_LEVEL_CRITICAL,
         PW_LOG_DEFAULT_FLAGS,
         "Crash: PW_ASSERT() or PW_DASSERT() failure");
#else
  PW_LOG(PW_LOG_LEVEL_CRITICAL,
         PW_LOG_DEFAULT_FLAGS,
         "Crash: PW_ASSERT() failure. Note: PW_DASSERT disabled");
#endif  // PW_ASSERT_ENABLE_DEBUG
  PW_UNREACHABLE;
}
