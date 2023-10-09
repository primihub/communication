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
#ifndef NETWORK_MEM_CHANNEL_H_
#define NETWORK_MEM_CHANNEL_H_

#include "common/threadsafe_queue.h"
#include "network/base_channel.h"

#include <map>
#include <mutex>
#include <string_view>

namespace primihub::link {
using ThreadSafeQueuePtr = std::shared_ptr<ThreadSafeQueue<std::string>>;
class MemoryChannel : public ChannelBase {
public:
  MemoryChannel() = default;
  MemoryChannel(const std::string &key);
  retcode SendImpl(const std::string &send_buf) override;
  retcode SendImpl(std::string_view send_buff_sv) override;
  retcode SendImpl(const char *buff, size_t size) override;
  retcode RecvImpl(std::string *recv_buf) override;
  retcode RecvImpl(char *recv_buf, size_t recv_size) override;
  std::shared_ptr<ChannelBase> ForkImpl(const std::string &key) override;
  void SetKey(const std::string &key) override;
  void close() override;
  void cancel() override;

private:
  ThreadSafeQueuePtr storage;
  std::string key_{"default"};
};
} // namespace primihub::link

#endif // NETWORK_MEM_CHANNEL_H_
