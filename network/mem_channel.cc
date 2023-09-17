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
#include <iostream>
#include <string_view>
#include <cstring>

namespace primihub::link {
retcode MemoryChannel::SendImpl(const std::string& send_buf) {
  auto send_sv = std::string_view(send_buf.data(), send_buf.size());
  SendImpl(send_sv);
  return retcode::SUCCESS;
}
retcode MemoryChannel::SendImpl(std::string_view send_buff_sv) {
  this->storage.clear();
  this->storage.append(send_buff_sv.data(), send_buff_sv.size());
  return retcode::SUCCESS;
}

retcode MemoryChannel::SendImpl(const char* buff, size_t size) {
  auto send_sv = std::string_view(buff, size);
  SendImpl(send_sv);
  return retcode::SUCCESS;
}

retcode MemoryChannel::RecvImpl(std::string* recv_buf) {
  *recv_buf = this->storage;
  return retcode::SUCCESS;
}

retcode MemoryChannel::RecvImpl(char* recv_buf, size_t recv_size) {
  memcpy(recv_buf, this->storage.data(), this->storage.size());
  return retcode::SUCCESS;
}
void MemoryChannel::close() {

}
void MemoryChannel::cancel() {

}
}  // namespace primihub::link