#include <glog/logging.h>
#include <gtest/gtest.h>

#include <array>
#include <iostream>
#include <vector>

#include "network/channel_interface.h"
#include "network/mem_channel.h"

using primihub::link::Channel;
using primihub::link::MemoryChannel;
using primihub::link::retcode;
using primihub::link::Status;

static std::string gen_random(uint32_t len, uint32_t seed) {
  static const char alphanum[] = "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz";
  std::string tmp_s;
  tmp_s.reserve(len);
  
  srand(seed);
  for (uint32_t i = 0; i < len; ++i)
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

  return tmp_s;
}

TEST(channel, type_test) {
  auto channel_impl = std::make_shared<MemoryChannel>();
  auto channel = std::make_shared<Channel>(channel_impl, "type_test");

  std::string buf = "Hello World";
  channel->asyncSend(buf);
  std::string recv_buf;
  channel->asyncRecv(recv_buf).get();

  EXPECT_EQ(buf, recv_buf);

  std::array<int64_t, 2> shape{1, 1};
  channel->asyncSend(shape);

  std::array<int64_t, 2> recv_shape;
  auto fut = channel->asyncRecv(recv_shape);
  fut.get();

  EXPECT_EQ(shape[0], recv_shape[0]);
  EXPECT_EQ(shape[1], recv_shape[1]);

  std::vector<int> vec{1, 2, 3, 4};
  channel->asyncSend(vec);

  std::vector<int> recv_vec;
  recv_vec.resize(vec.size());
  channel->asyncRecv(recv_vec).get();

  EXPECT_EQ(vec.size(), recv_vec.size());
  for (size_t i = 0; i < recv_vec.size(); i++)
    EXPECT_EQ(vec[i], recv_vec[i]);
}

TEST(channel, multiple_test) {
  std::string str1 = gen_random(1024, 1);
  std::string str2 = gen_random(2048, 2);
  std::string str3 = gen_random(4096, 3);

  auto channel_impl = std::make_shared<MemoryChannel>();
  auto channel = std::make_shared<Channel>(channel_impl, "type_test");

  auto status = channel->asyncSend(str1);
  EXPECT_EQ(status.IsOK(), true);

  status = channel->asyncSend(str2);
  EXPECT_EQ(status.IsOK(), true);

  status = channel->asyncSend(str3);
  EXPECT_EQ(status.IsOK(), true);

  std::string recv_str1;
  std::string recv_str2;
  std::string recv_str3;

  {
    auto fut = channel->asyncRecv(recv_str1);
    fut.get();
  }

  {
    auto fut = channel->asyncRecv(recv_str2);
    fut.get();
  }

  {
    auto fut = channel->asyncRecv(recv_str3);
    fut.get();
  }

  EXPECT_EQ(recv_str1, str1);
  EXPECT_EQ(recv_str2, str2);
  EXPECT_EQ(recv_str3, str3);
}

TEST(channel, fork_test) {
  uint16_t fork_num = 10;
  std::vector<std::shared_ptr<Channel>> client_fork_channels;
  std::vector<std::shared_ptr<Channel>> server_fork_channels;

  auto channel_impl = std::make_shared<MemoryChannel>();
  auto channel = std::make_shared<Channel>(channel_impl, "fork_test");

  for (uint16_t i = 0; i < fork_num; i++) {
    auto new_channel = channel->fork();
    client_fork_channels.push_back(new_channel);
    server_fork_channels.push_back(new_channel);
  }

  std::string send_buf = gen_random(1024, 10);
  auto send_fn = [&client_fork_channels, &send_buf]() {
    std::vector<Status> send_status;

    for (auto &channel : client_fork_channels)
      send_status.push_back(channel->asyncSend(send_buf.data(), send_buf.size()));

    for (auto &status : send_status)
      EXPECT_EQ(status.IsOK(), true);
  };

  auto recv_fn = [&server_fork_channels, &send_buf]() {
    std::vector<std::string> all_recv_buf;
    all_recv_buf.resize(server_fork_channels.size());
    for (auto &recv_buf : all_recv_buf)
      recv_buf.resize(1024);

    std::vector<std::future<Status>> recv_futs;
    for (size_t i = 0; i < server_fork_channels.size(); i++)
      recv_futs.push_back(server_fork_channels[i]->asyncRecv(
          all_recv_buf[i].data(), all_recv_buf[i].size()));

    for (auto &fut : recv_futs)
      EXPECT_EQ(fut.get().IsOK(), true);

    for (auto &recv_buf : all_recv_buf)
      EXPECT_EQ(recv_buf == send_buf, true);
  };

  std::future<void> recv_fut = std::async(recv_fn);
  std::future<void> send_fut = std::async(send_fn);

  send_fut.get();
  recv_fut.get();
}
