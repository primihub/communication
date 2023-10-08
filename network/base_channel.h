/*
 * Copyright (c) 2023 by PrimiHub
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef NETWORK_BASE_CHANNEL_H_
#define NETWORK_BASE_CHANNEL_H_
#include "common/common.h"
#include <glog/logging.h>
#include <string>
#include <string_view>
#include <memory>

namespace primihub::link {
class ChannelBase {
public:
  ChannelBase() = default;
  virtual ~ChannelBase() = default;
  virtual retcode SendImpl(const std::string &send_buf) = 0;
  virtual retcode SendImpl(std::string_view send_buff_sv) = 0;
  virtual retcode SendImpl(const char *buff, size_t size) = 0;
  virtual retcode RecvImpl(std::string *recv_buf) = 0;
  virtual retcode RecvImpl(char *recv_buf, size_t recv_size) = 0;
  virtual std::shared_ptr<ChannelBase> ForkImpl(const std::string &key) {
    LOG(ERROR) << "Not implement error.";
    return nullptr;
  }

  virtual void SetKey(const std::string &key) {}

  virtual void close() = 0;
  virtual void cancel() = 0;
};
} // namespace primihub::link
#endif // NETWORK_BASE_CHANNEL_H_
