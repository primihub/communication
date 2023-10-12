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
#ifndef NETWORK_CHANNEL_INTERFACE_H_
#define NETWORK_CHANNEL_INTERFACE_H_
#include <glog/logging.h>

#include "network/base_channel.h"
#include "network/status.h"
#include "util/type_trait.h"
#include <cassert>
#include <cstring>
#include <future>
#include <iostream>
#include <thread>
#include <map>
#include <mutex>

namespace primihub::link {
// Channel is the standard interface use to send data over the network.
class Channel : public std::enable_shared_from_this<Channel> {
public:
  // The default constructors
  Channel() = default;
  Channel(std::shared_ptr<ChannelBase> channel_impl)
      : channel_impl_(std::move(channel_impl)) {}
  Channel(const Channel &copy) {
    this->channel_impl_ = copy.channel_impl_;
    this->key_ = copy.key_;
    this->sended_data_ = 0;
    this->received_data_ = 0;
    this->num_fork_ = copy.num_fork_;
  }

  Channel(std::shared_ptr<ChannelBase> channel_impl, const std::string &key) {
    this->channel_impl_ = channel_impl;
    this->key_ = key;
    this->sended_data_ = 0;
    this->received_data_ = 0;
    this->num_fork_ = 0;
    channel_impl->SetKey(key);
  }

  Channel(Channel &&move) = default;

  virtual ~Channel() = default;

  std::shared_ptr<Channel> fork(void) {
    num_fork_++;
    std::string new_key = key_ + "_fork_" + std::to_string(num_fork_);

    std::shared_ptr<ChannelBase> base = channel_impl_->ForkImpl(new_key);
    std::shared_ptr<Channel> new_channel =
        std::make_shared<Channel>(base, new_key);

    return new_channel;
  }

  std::string getKey(void) { return key_; }

  // Default assignment
  Channel &operator=(Channel &&move_ins) {
    *this = std::move(move_ins);
    return *this;
  }

  // Default assignment
  Channel &operator=(const Channel &copy) {
    this->channel_impl_ = copy.channel_impl_;
    this->sended_data_ = 0;
    this->received_data_ = 0;
    return *this;
  }

  //////////////////////////////////////////////////////////////////////////////
  //						   Sending interface
  ////
  //////////////////////////////////////////////////////////////////////////////

  // Sends length number of T pointed to by src over the network. The type T
  // must be POD. Returns once all the data has been sent.
  template <typename T>
  typename std::enable_if<std::is_pod<T>::value, Status>::type
  send(const T *src, uint64_t length);

  // Sends the data in buf over the network. The type Container  must meet the
  // requirements defined in IoBuffer.h. Returns once all the data has been
  // sent.
  template <class T>
  typename std::enable_if<std::is_pod<T>::value, Status>::type
  send(const T &buf);

  // Sends the data in buf over the network. The type Container  must meet the
  // requirements defined in IoBuffer.h. Returns once all the data has been
  // sent.
  template <class Container>
  typename std::enable_if<is_container<Container>::value, Status>::type
  send(const Container &buf);

  // Sends the data in buf over the network. The type T must be POD.
  // Returns before the data has been sent. The life time of the data must be
  // managed externally to ensure it lives longer than the async operations.
  template <typename T>
  typename std::enable_if<std::is_pod<T>::value, Status>::type
  asyncSend(const T *data, uint64_t length);

  // // Sends the data in buf over the network. The type Container  must meet
  // the
  // // requirements defined in IoBuffer.h. Returns before the data has been
  // sent.
  // // The life time of the data must be managed externally to ensure it lives
  // // longer than the async operations.  callback is a function that is called
  // // from another thread once the send operation has succeeded.
  // template<typename Container>
  // typename std::enable_if<is_container<Container>::value, void>::type
  //     asyncSend(Container&& data, std::function<void()> callback);

  // Sends the data in buf over the network. The type T must be POD.
  // Returns before the data has been sent. The life time of the data must be
  // managed externally to ensure it lives longer than the async operations.
  template <typename T>
  typename std::enable_if<std::is_pod<T>::value, Status>::type
  asyncSend(const T &data);

  // Sends the data in buf over the network. The type T must be POD.
  // Returns before the data has been sent. The life time of the data must be
  // managed externally to ensure it lives longer than the async operations.
  template <typename Container>
  typename std::enable_if<is_container<Container>::value, Status>::type
  asyncSend(const Container &data);

  // Sends the data in buf over the network. The type Container  must meet the
  // requirements defined in IoBuffer.h. Returns before the data has been sent.
  template <class Container>
  typename std::enable_if<is_container<Container>::value, Status>::type
  asyncSend(Container &&c);

  // Sends the data in buf over the network. The type Container  must meet the
  // requirements defined in IoBuffer.h. Returns before the data has been sent.
  template <class Container>
  typename std::enable_if<is_container<Container>::value, Status>::type
  asyncSend(std::unique_ptr<Container> buffer);

  // Sends the data in buf over the network. The type Container  must meet the
  // requirements defined in IoBuffer.h. Returns before the data has been sent.
  template <class Container>
  typename std::enable_if<is_container<Container>::value, Status>::type
  asyncSend(std::shared_ptr<Container> buffer);

  // Sends the data in buf over the network. The type T must be POD.
  // Returns before the data has been sent. The life time of the data must be
  // managed externally to ensure it lives longer than the async operations.
  template <typename T>
  typename std::enable_if<std::is_pod<T>::value, std::future<Status>>::type
  asyncSendFuture(const T *data, uint64_t length);

  // Performs a data copy and then sends the data in buf over the network.
  //  The type T must be POD. Returns before the data has been sent.
  template <typename T>
  typename std::enable_if<std::is_pod<T>::value, Status>::type
  asyncSendCopy(const T &buff);

  // Performs a data copy and then sends the data in buf over the network.
  //  The type T must be POD. Returns before the data has been sent.
  template <typename T>
  typename std::enable_if<std::is_pod<T>::value, Status>::type
  asyncSendCopy(const T *bufferPtr, uint64_t length);

  // Performs a data copy and then sends the data in buf over the network.
  // The type Container must meet the requirements defined in IoBuffer.h.
  // Returns before the data has been sent.
  template <typename Container>
  typename std::enable_if<is_container<Container>::value, Status>::type
  asyncSendCopy(const Container &buf);

  //////////////////////////////////////////////////////////////////////////////
  //						   Receiving interface
  ////
  //////////////////////////////////////////////////////////////////////////////

  // Receive data over the network. If possible, the container c will be resized
  // to fit the data. The function returns once all the data has been received.
  template <class Container>
  typename std::enable_if<
      is_container<Container>::value &&
          has_resize<Container, void(typename Container::size_type)>::value,
      Status>::type
  recv(Container &c) {
    return asyncRecv(c).get();
  }

  // Receive data over the network. The container c must be the correct size to
  // fit the data. The function returns once all the data has been received.
  template <class Container>
  typename std::enable_if<
      is_container<Container>::value &&
          !has_resize<Container, void(typename Container::size_type)>::value,
      Status>::type
  recv(Container &c) {
    return asyncRecv(c).get();
  }

  // Receive data over the network. The function returns once all the data
  // has been received.
  template <typename T>
  typename std::enable_if<std::is_pod<T>::value, Status>::type
  recv(T *dest, uint64_t length);

  // Receive data over the network. The function returns once all the data
  // has been received.
  template <typename T>
  typename std::enable_if<std::is_pod<T>::value, Status>::type recv(T &dest) {
    return recv(&dest, 1);
  }

  // Receive data over the network asynchronously.
  // The function returns right away, before the data has been received.
  //  When all the data has benn received the future is set.
  template <typename T>
  typename std::enable_if<std::is_pod<T>::value, std::future<Status>>::type
  asyncRecv(T *dest, uint64_t length);

  // // Receive data over the network asynchronously.
  // // The function returns right away, before the data has been received.
  // // When all the data has benn received the
  // // future is set and the callback fn is called.
  // template<typename T>
  // typename std::enable_if<std::is_pod<T>::value, std::future<void>>::type
  // asyncRecv(T* dest, uint64_t length, std::function<void()> fn);

  // Receive data over the network asynchronously.
  // The function returns right away, before the data has been received.
  // When all the data has benn received the future is set.
  template <typename T>
  typename std::enable_if<std::is_pod<T>::value, std::future<Status>>::type
  asyncRecv(T &dest) {
    return asyncRecv(&dest, 1);
  }

  // Receive data over the network asynchronously.
  // The function returns right away, before the data has been received.
  // When all the data has benn received the future is set.
  // The container must be the correct size to fit the data received.
  template <class Container>
  typename std::enable_if<
      is_container<Container>::value &&
          !has_resize<Container, void(typename Container::size_type)>::value,
      std::future<Status>>::type
  asyncRecv(Container &c);

  // Receive data over the network asynchronously.
  // The function returns right away, before the data has been received.
  // When all the data has benn received the
  // future is set. The container is resized to fit the data.
  // template <class Container>
  // typename std::enable_if<
  //     is_container<Container>::value &&
  //         has_resize<Container, void(typename Container::size_type)>::value,
  //     std::future<Status>>::type
  // asyncRecv(Container & c);

  template <class Container>
  typename std::enable_if<
      is_container<Container>::value &&
          has_resize<Container, void(typename Container::size_type)>::value &&
          std::is_same_v<Container, std::string>,
      std::future<Status>>::type
  asyncRecv(Container &c);

  template <class Container>
  typename std::enable_if<
      is_container<Container>::value &&
          has_resize<Container, void(typename Container::size_type)>::value &&
          !std::is_same_v<Container, std::string>,
      std::future<Status>>::type
  asyncRecv(Container &c);

  // Receive data over the network asynchronously.
  // The function returns right away, before the data has been received.
  // When all the data has benn received the
  // future is set and the callback fn is called.
  // The container must be the correct size to fit the data received.
  // template <class Container>
  // typename std::enable_if<
  //     is_container<Container>::value &&
  //     has_resize<Container, void(typename Container::size_type)>::value,
  //                std::future<Status>>::type
  // asyncRecv(Container & c, std::function<void()> fn);

  //////////////////////////////////////////////////////////////////////////////
  //						   Utility functions
  ////
  //////////////////////////////////////////////////////////////////////////////

  // Returns the amount of data that this channel has sent since it was created
  // or when resetStats() was last called.
  uint64_t getTotalDataSent() const { return sended_data_.load(); }

  // Returns the amount of data that this channel has sent since it was created
  // or when resetStats() was last called.
  uint64_t getTotalDataRecv() const { return received_data_.load(); }

  // Close this channel to denote that no more data will be sent or received.
  // blocks until all pending operations have completed.
  void close() { channel_impl_->close(); }

  // Aborts all current operations (connect, send, receive).
  void cancel(bool close = true) { channel_impl_->cancel(); }

private:
  std::shared_ptr<ChannelBase> channel_impl_;
  std::atomic<uint64_t> sended_data_{0};
  std::atomic<uint64_t> received_data_{0};
  std::string key_{"default"};
  uint32_t num_fork_{0};
};

template <typename T> inline char *BuffData(const T &container) {
  // return reinterpret_cast<char*>(
  //           reinterpret_cast<const T::value_type*>(container.data()));
  return (char *)(container.data());
}

template <typename T> inline uint64_t BuffSize(const T &container) {
  return container.size() * sizeof(typename T::value_type);
}

template <class Container>
typename std::enable_if<is_container<Container>::value, Status>::type
Channel::asyncSend(std::unique_ptr<Container> c) {
  return send(BuffData(*c), BuffSize(*c));
}

template <class Container>
typename std::enable_if<is_container<Container>::value, Status>::type
Channel::asyncSend(std::shared_ptr<Container> c) {
  return send(BuffData(*c), BuffSize(*c));
}

template <class Container>
typename std::enable_if<is_container<Container>::value, Status>::type
Channel::asyncSend(const Container &c) {
  return send(BuffData(c), BuffSize(c));
}

template <class Container>
typename std::enable_if<is_container<Container>::value, Status>::type
Channel::asyncSend(Container &&c) {
  auto send_c = std::move(c);
  return send(BuffData(send_c), BuffSize(send_c));
}

template <class Container>
typename std::enable_if<
    is_container<Container>::value &&
        !has_resize<Container, void(typename Container::size_type)>::value,
    std::future<Status>>::type
Channel::asyncRecv(Container &c) {
  auto recv_func = [&](Container &c) -> Status {
    retcode ret = this->channel_impl_->RecvImpl(BuffData(c), BuffSize(c));
    if (ret == retcode::SUCCESS)
      return Status::OK();
    else
      return Status::NetworkError();
  };
  return std::async(std::launch::async, recv_func, std::ref(c));
}

// template <class Container>
// typename std::enable_if<
//     is_container<Container>::value &&
//         has_resize<Container, void(typename Container::size_type)>::value,
//     std::future<Status>>::type
// Channel::asyncRecv(Container& c) {
//   auto recv_func = [&](Container& c) -> Status {
//     std::string recv_buf;
//     this->channel_impl_->RecvImpl(&recv_buf);
//     if (BuffSize(c) != recv_buf.size()) {
//       LOG(WARNING) << "size does not match, "
//                    << "need resize to " << recv_buf.size();
//       using value_type_t = typename Container::value_type;
//       size_t item_size = recv_buf.size()/sizeof(value_type_t);
//       c.resize(item_size);
//     }
//     memcpy(BuffData(c), recv_buf.data(), recv_buf.size());
//     return Status::OK();
//   };
//   return std::async(std::launch::async, recv_func, std::ref(c));
// }

template <class Container>
typename std::enable_if<
    is_container<Container>::value &&
        has_resize<Container, void(typename Container::size_type)>::value &&
        !std::is_same_v<Container, std::string>,
    std::future<Status>>::type
Channel::asyncRecv(Container &c) {
  auto recv_func = [&](Container &c) -> Status {
    std::string recv_buf;
    this->channel_impl_->RecvImpl(&recv_buf);
    if (BuffSize(c) != recv_buf.size()) {
      LOG(WARNING) << "size does not match, "
                   << "need resize to " << recv_buf.size();
      using value_type_t = typename Container::value_type;
      size_t item_size = recv_buf.size() / sizeof(value_type_t);
      c.resize(item_size);
    }
    memcpy(BuffData(c), recv_buf.data(), recv_buf.size());
    return Status::OK();
  };
  return std::async(std::launch::async, recv_func, std::ref(c));
}

template <class Container>
typename std::enable_if<
    is_container<Container>::value &&
        has_resize<Container, void(typename Container::size_type)>::value &&
        std::is_same_v<Container, std::string>,
    std::future<Status>>::type
Channel::asyncRecv(Container &c) {
  auto recv_func = [&](Container &c) -> Status {
    this->channel_impl_->RecvImpl(&c);
    return Status::OK();
  };
  return std::async(std::launch::async, recv_func, std::ref(c));
}

// template <class Container>
// typename std::enable_if<
//     is_container<Container>::value&&
//     has_resize<Container, void(typename Container::size_type)>::value,
//                std::future<Status>>::type
// Channel::asyncRecv(Container& c, std::function<void()> fn) {
//   auto recv_func = [&](Container& c, std::function<void()> done) -> Status {
//     std::string recv_buf;
//     LOG(ERROR) << "Channel::asyncRecv";
//     this->channel_impl_->RecvImpl(&recv_buf);
//     if (BuffSize(c) != recv_buf.size()) {
//       using value_type_t = typename Container::value_type;
//       size_t item_size = recv_buf.size()/sizeof(value_type_t);
//       c.resize(item_size);
//     }
//     memcpy(BuffData(c), recv_buf.data(), recv_buf.size());
//     done();
//   };
//   return std::async(std::launch::async, recv_func, std::ref(c), fn);
// }

template <class Container>
typename std::enable_if<is_container<Container>::value, Status>::type
Channel::send(const Container &buf) {
  return send(BuffData(buf), BuffSize(buf));
}

template <typename Container>
typename std::enable_if<is_container<Container>::value, Status>::type
Channel::asyncSendCopy(const Container &buf) {
  return asyncSend(Container(buf));
}

template <typename T>
typename std::enable_if<std::is_pod<T>::value, Status>::type
Channel::send(const T *buffT, uint64_t sizeT) {
  char *buff = reinterpret_cast<char *>(const_cast<T *>(buffT));
  auto buff_length = sizeT * sizeof(T);
  this->channel_impl_->SendImpl(buff, buff_length);
  return Status::OK();
}

template <typename T>
typename std::enable_if<std::is_pod<T>::value, std::future<Status>>::type
Channel::asyncSendFuture(const T *buffT, uint64_t sizeT) {
  char *buff = reinterpret_cast<char *>(const_cast<T *>(buffT));
  auto buff_length = sizeT * sizeof(T);
  auto send_func = [&](char *buff, uint64_t length) -> Status {
    this->channel_impl_->SendImpl(buff, length);
    return Status::OK();
  };
  return std::async(std::launch::async, send_func, buff, buff_length);
}

template <typename T>
typename std::enable_if<std::is_pod<T>::value, Status>::type
Channel::send(const T &buffT) {
  return send(&buffT, 1);
}

template <typename T>
typename std::enable_if<std::is_pod<T>::value, std::future<Status>>::type
Channel::asyncRecv(T *buffT, uint64_t sizeT) {
  char *buff = reinterpret_cast<char *>(buffT);
  auto size = sizeT * sizeof(T);
  auto recv_func = [&](char *buf, uint64_t length) -> Status {
    retcode ret = this->channel_impl_->RecvImpl(buf, length);
    if (ret == retcode::SUCCESS)
      return Status::OK();
    else 
      return Status::NetworkError();
  };
  return std::async(std::launch::async, recv_func, buff, size);
}

template <typename T>
typename std::enable_if<std::is_pod<T>::value, Status>::type
Channel::asyncSend(const T *buffT, uint64_t sizeT) {
  char *buff = reinterpret_cast<char *>(const_cast<T *>(buffT));
  uint64_t size = sizeT * sizeof(T);
  channel_impl_->SendImpl(buff, size);
  return Status::OK();
}

template <typename T>
typename std::enable_if<std::is_pod<T>::value, Status>::type
Channel::asyncSend(const T &v) {
  return asyncSend(&v, 1);
}

// template<class Container>
// typename std::enable_if<is_container<Container>::value, void>::type
// Channel::asyncSend(Container&& c, std::function<void()> callback) {
//   send(c);
//   callback();
// }

template <typename T>
typename std::enable_if<std::is_pod<T>::value, Status>::type
Channel::recv(T *buff, uint64_t size) {
  char *recv_buf = reinterpret_cast<char *>(buff);
  uint64_t length = sizeof(T) * size;
  channel_impl_->RecvImpl(recv_buf, length);
  return Status::OK();
}

template <typename T>
typename std::enable_if<std::is_pod<T>::value, Status>::type
Channel::asyncSendCopy(const T *bufferPtr, uint64_t size) {
  char *send_buf = reinterpret_cast<char *>(const_cast<T *>(bufferPtr));
  uint64_t length = sizeof(T) * size;
  auto send_sv = std::string_view(send_buf, length);
  channel_impl_->SendImpl(send_sv);
  return Status::OK();
}

template <typename T>
typename std::enable_if<std::is_pod<T>::value, Status>::type
Channel::asyncSendCopy(const T &buf) {
  return asyncSendCopy(&buf, 1);
}

} // namespace primihub::link

#endif // NETWORK_CHANNEL_INTERFACE_H_
