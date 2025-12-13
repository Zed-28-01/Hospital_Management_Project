#!/bin/bash

echo "--- BAT DAU CAI DAT TU DONG ---"

# 1. Cap nhat he thong va cai dat GCC-14 (Theo anh image_16d896.png)
sudo apt-get update
sudo apt-get install -y software-properties-common
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install -y g++-14 gcc-14

# 2. Thiet lap GCC-14 lam mac dinh (uu tien 100)
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 100

# 3. Cai dat Google Test & CMake (Theo anh image_16d511.png)
sudo apt-get install -y libgtest-dev cmake
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp *.a /usr/lib

echo "--- CAI DAT HOAN TAT! ---"
g++ --version