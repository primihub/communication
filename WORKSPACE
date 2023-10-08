
workspace(name = "communication")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

http_archive(
  name = "com_github_glog_glog",
  # sha256 = "cbba86b5a63063999e0fc86de620a3ad22d6fd2aa5948bff4995dcd851074a0b",
  strip_prefix = "glog-0.6.0",
  urls = ["https://primihub.oss-cn-beijing.aliyuncs.com/tools/glog-0.6.0.zip"],
)

# gflags Needed for glog
http_archive(
  name = "com_github_gflags_gflags",
  sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
  strip_prefix = "gflags-2.2.2",
  urls = [
    "https://primihub.oss-cn-beijing.aliyuncs.com/tools/gflags-2.2.2.tar.gz",
  ],
)

http_archive(
  name = "com_google_googletest",
  urls = ["https://primihub.oss-cn-beijing.aliyuncs.com/tools/googletest-release-1.10.0.zip"],
  sha256 = "94c634d499558a76fa649edb13721dce6e98fb1e7018dfaeba3cd7a083945e91",
  strip_prefix = "googletest-release-1.10.0",
)
