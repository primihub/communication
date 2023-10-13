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
class QueuePair {
public:
  QueuePair() {
    this->queue_c2s_ = std::make_shared<ThreadSafeQueue<std::string>>();
    this->queue_s2c_ = std::make_shared<ThreadSafeQueue<std::string>>();
  }

  ThreadSafeQueuePtr getQueue(bool c2s) {
    if (c2s)
      return queue_c2s_;
    else
      return queue_s2c_;
  }

private:
  ThreadSafeQueuePtr queue_c2s_;
  ThreadSafeQueuePtr queue_s2c_;
};

class QueueManager {
public:
  QueueManager() = default;
  std::shared_ptr<QueuePair> getOrCreate(const std::string &key) {
    std::lock_guard<std::mutex> lock(map_mu_);
    auto iter = queue_map_.find(key);
    if (iter != queue_map_.end())
      return iter->second;

    std::shared_ptr<QueuePair> queue = std::make_shared<QueuePair>();
    queue_map_.insert(std::make_pair(key, queue));

    return queue;
  }

private:
  std::map<std::string, std::shared_ptr<QueuePair>> queue_map_;
  std::mutex map_mu_;
};

QueueManager manager;
} // namespace

MemoryChannel::MemoryChannel(MemoryChannel::ChannelRole role) {
  this->role_ = role;
}

MemoryChannel::MemoryChannel(const std::string &key,
                             MemoryChannel::ChannelRole role) {
  this->key_ = key;
  this->role_ = role;

  std::shared_ptr<QueuePair> queue = manager.getOrCreate(this->key_);
  storage_c2s_ = queue->getQueue(true);
  storage_s2c_ = queue->getQueue(false);
}

void MemoryChannel::SetKey(const std::string &key) {
  this->key_ = key;

  std::shared_ptr<QueuePair> queue = manager.getOrCreate(this->key_);
  storage_c2s_ = queue->getQueue(true);
  storage_s2c_ = queue->getQueue(false);
}

retcode MemoryChannel::SendImpl(std::string_view send_buff_sv) {
  ThreadSafeQueuePtr storage = nullptr;
  if (role_ == ChannelRole::SERVER)
    storage = storage_s2c_;
  else
    storage = storage_c2s_;

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
  ThreadSafeQueuePtr storage = nullptr;
  if (role_ == ChannelRole::SERVER)
    storage = storage_c2s_;
  else
    storage = storage_s2c_;

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
  ThreadSafeQueuePtr storage = nullptr;
  if (role_ == ChannelRole::SERVER)
    storage = storage_c2s_;
  else
    storage = storage_s2c_;

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
  return std::make_shared<MemoryChannel>(key, this->role_);
}

void MemoryChannel::close() {}

void MemoryChannel::cancel() {}

} // namespace primihub::link
