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
#ifndef NETWORK_STATUS_H_
#define NETWORK_STATUS_H_
namespace primihub::link {
class Status {
private:
  enum class Code {
    kOK = 0,
    kNetworkError,
    kMismatchError,
    kTimeoutError,
    kDuplicateError,
    kNotFoundError,
    kSyscallError,
    kInvalidError,
    kNotImplementError,
    kUnavailableError,
  };

public:
  virtual ~Status() = default;

  Status(const Status &rhs) = delete;
  Status& operator=(const Status &rhs) = delete;
  Status Copy() const { return Status(status_code_); }
  Status(Status &&rhs) = default;
  Status& operator=(Status &&rhs) = default;

  static Status OK() { return Status(Code::kOK); }
  static Status NetworkError() { return Status(Code::kNetworkError); }
  static Status MismatchError() { return Status(Code::kMismatchError); }
  static Status TimeoutError() { return Status(Code::kTimeoutError); }
  static Status DuplicateError() { return Status(Code::kDuplicateError); }
  static Status NotFoundError() { return Status(Code::kNotFoundError); }
  static Status SyscallError() { return Status(Code::kSyscallError); }
  static Status InvalidError() { return Status(Code::kInvalidError); }
  static Status NotImplementError() { return Status(Code::kNotImplementError); }
  static Status UnavailableError() { return Status(Code::kUnavailableError); }

  bool IsOK() const { return status_code_ == Code::kOK; }

private:
  explicit Status(const Code &status_code) : status_code_(status_code) {}
  Code status_code_;
};
} // namespace primihub::link
#endif  // NETWORK_STATUS_H_
