target = //test:main

release:
	bazel build --cxxopt=-std=c++17 ${target}

.PHONY: clean

clean:
	bazel clean
