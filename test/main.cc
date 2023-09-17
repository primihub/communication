#include <glog/logging.h>

#include <iostream>
#include <vector>
#include <array>
#include "network/mem_channel.h"
#include "network/channel_interface.h"
int main() {
  // create channel
  auto channel_impl = std::make_shared<primihub::link::MemoryChannel>();
  auto channel = std::make_shared<primihub::link::Channel>(channel_impl);
  // test send recv string
  LOG(INFO) << "test send recv string";
  std::string buf = "Hello World";
  channel->asyncSend(buf);
  std::string recv_buf;
  channel->asyncRecv(recv_buf).get();
  LOG(INFO) << "recv string content: " << recv_buf;
  // test send recv arary
  LOG(INFO) << "test send recv arary";
  std::array<int64_t, 2> shape{1, 1};
  std::string send_item;
  for (const auto& item: shape) {
    send_item.append(std::to_string(item)).append(" ");
  }
  LOG(INFO) << "origin item: " << send_item;
  channel->asyncSend(shape);
  std::array<int64_t, 2> recv_shape;
  auto fut = channel->asyncRecv(recv_shape);
  fut.get();
  std::string recv_item;
  for (const auto& item : recv_shape) {
    recv_item.append(std::to_string(item)).append(" ");
  }
  LOG(INFO) << "recv array item: " << recv_item;

  // test send recv vector
  LOG(INFO) << "test send recv vector";
  std::vector<int> vec{1, 2, 3, 4};
  std::string send_vec_item;
  for (const auto& item: vec) {
    send_vec_item.append(std::to_string(item)).append(" ");
  }
  LOG(INFO) << "origin vector item: " << send_vec_item;
  channel->asyncSend(vec);
  std::vector<int> recv_vec;
  recv_vec.resize(vec.size());
  channel->asyncRecv(recv_vec).get();
  std::string recv_vec_item;
  for (const auto& item : recv_vec) {
    recv_vec_item.append(std::to_string(item)).append(" ");
  }
  LOG(INFO) << "recv vector item: " << recv_vec_item;
  return 0;
}