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
#ifndef UTIL_TYPE_TRAIT_H_
#define UTIL_TYPE_TRAIT_H_
#include <memory>
#include <utility>

namespace primihub {
/// type trait that defines what is considered a STL like Container
///
/// Must have the following member types:  pointer, size_type, value_type
/// Must have the following member functions:
///    * Container::pointer Container::data();
///    * Container::size_type Container::size();
/// Must contain Plain Old Data:
///    * std::is_pod<Container::value_type>::value == true
template<typename Container>
using is_container =
    std::is_same<typename std::enable_if<
    std::is_convertible<
    typename Container::pointer,
    decltype(std::declval<Container>().data())>::value&&
    std::is_convertible<
    typename Container::size_type,
    decltype(std::declval<Container>().size())>::value&&
    std::is_pod<typename Container::value_type>::value&&
    std::is_pod<Container>::value == false>::type
    ,
    void>;

template<typename, typename T>
struct has_resize {
  static_assert(
    std::integral_constant<T, false>::value,
    "Second template parameter needs to be of function type.");
};

// specialization that does the checking
template<typename C, typename Ret, typename... Args>
struct has_resize<C, Ret(Args...)> {
 private:
  // attempt to call it and see if the return type is correct
  template<typename T>
  static constexpr auto check(T*)
      -> typename
      std::is_same<
      decltype(std::declval<T>().resize(std::declval<Args>()...)), Ret>::type;

  template<typename>
  static constexpr std::false_type check(...);

  typedef decltype(check<C>(0)) type;

public:
    static constexpr bool value = type::value;
};

}  // namespace primihub
#endif  // UTIL_TYPE_TRAIT_H_
