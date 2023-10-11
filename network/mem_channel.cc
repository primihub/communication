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
#include "network/mem_channel.h"
#include "common/threadsafe_queue.h"

#include <cstring>
#include <iostream>
#include <string_view>

namespace primihub::link {
namespace {
class QueueManager {
public:
  QueueManager() = default;
  ThreadSafeQueuePtr getOrCreate(const std::string &key) {
    std::lock_guard<std::mutex> lock(map_mu_);
    auto iter = queue_map_.find(key);
    if (iter != queue_map_.end())
      return iter->second;

    std::shared_ptr<ThreadSafeQueue<std::string>> queue =
        std::make_shared<ThreadSafeQueue<std::string>>();
    queue_map_.insert(std::make_pair(key, queue));

    return queue;
  }

private:
  std::map<std::string, ThreadSafeQueuePtr> queue_map_;
  std::mutex map_mu_;
};

QueueManager manager;
} // namespace

MemoryChannel::MemoryChannel(const std::string &key) { 
  this->key_ = key; 
  storage = manager.getOrCreate(this->key_);
}

void MemoryChannel::SetKey(const std::string &key) {
  this->key_ = key;
  storage = manager.getOrCreate(this->key_);
}

retcode MemoryChannel::SendImpl(std::string_view send_buff_sv) {
  storage->push(std::string(send_buff_sv.data(), send_buff_sv.size()));

  if (VLOG_IS_ON(8)) {
    std::string send_data;
    for (const auto &ch : send_buff_sv) {
      send_data.append(std::to_string(static_cast<int>(ch))).append(" ");
    }
    LOG(INFO) << "MemoryChannel::SendImpl "
              << "send_key: " << key_ << " "
              << "data size: " << send_buff_sv.size();
              // << "send data: [" << send_data << "]";
  }

  return retcode::SUCCESS;
}

retcode MemoryChannel::SendImpl(const std::string &send_buf) {
  auto send_sv = std::string_view(send_buf.data(), send_buf.size());
  return SendImpl(send_sv);
}

retcode MemoryChannel::SendImpl(const char *buff, size_t size) {
  auto send_sv = std::string_view(buff, size);
  return SendImpl(send_sv);
}

retcode MemoryChannel::RecvImpl(std::string *recv_buf) {
  std::string data_buf;
  storage->wait_and_pop(data_buf);
  *recv_buf = std::move(data_buf);

  if (VLOG_IS_ON(8)) {
    std::string recv_data;
    for (const auto &ch : *recv_buf) {
      recv_data.append(std::to_string(static_cast<int>(ch))).append(" ");
    }

    LOG(INFO) << "MemoryChannel::RecvImpl "
              << "recv_key: " << key_ << " data size: " << recv_buf->size();
              // << " recv data: " << recv_data;
  }

  return retcode::SUCCESS;
}

retcode MemoryChannel::RecvImpl(char *recv_buf, size_t recv_size) {
  std::string tmp_recv_buf;
  storage->wait_and_pop(tmp_recv_buf);
  if (tmp_recv_buf.size() != recv_size) {
    LOG(ERROR) << "data length does not match: "
               << " "
               << "expected: " << recv_size << " "
               << "actually: " << tmp_recv_buf.size();
    return retcode::FAIL;
  }

  memcpy(recv_buf, tmp_recv_buf.data(), recv_size);

  if (VLOG_IS_ON(8)) {
    std::string recv_data;
    for (const auto &ch : tmp_recv_buf) {
      recv_data.append(std::to_string(static_cast<int>(ch))).append(" ");
    }

    LOG(INFO) << "MemoryChannel::RecvImpl "
               << "recv_key: " << key_ << " "
               << "data size: " << recv_size;
               // << "recv data: [" << recv_data << "] ";
  }

  return retcode::SUCCESS;
}

std::shared_ptr<ChannelBase> MemoryChannel::ForkImpl(const std::string &key) {
  return std::make_shared<MemoryChannel>(key);
}

void MemoryChannel::close() {}

void MemoryChannel::cancel() {}

} // namespace primihub::link
