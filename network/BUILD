package(default_visibility = ["//visibility:public"])
cc_library(
  name = "base_channel",
  hdrs = ["base_channel.h"],
  deps = [
    "//common:common_def"
  ],
)

cc_library(
  name = "channel_interface",
  hdrs = [
    "channel_interface.h",
    "status.h"
  ],
  srcs = ["channel_interface.cc"],
  linkopts = [
    "-lpthread",
  ],
  deps = [
    ":base_channel",
    "//util:type_trait",
    "@com_github_glog_glog//:glog",
  ],
)

cc_library(
  name = "mem_channel",
  hdrs = ["mem_channel.h"],
  srcs = ["mem_channel.cc"],
  deps = [
    ":base_channel",
  ],
)