# Multicast Publisher/Subscriber Demo

## Overview
This project provides ultra low-latency multicast publisher and subscriber binaries using C++ and raw UDP sockets. The publisher attaches a header to each payload, and the subscriber parses it.

## Build Instructions

### Prerequisites
- CMake >= 3.10
- GCC >= 9 (C++17 support)


### Build (Release/Optimized)
```
cd mcast-1
mkdir -p build-opt
cd build-opt
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Build (Optimized Debug / RelWithDebInfo)
```
cd mcast-1
mkdir -p build-opt-debug
cd build-opt-debug
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make -j$(nproc)
```

### Build (Debug)
```
cd mcast-1
mkdir -p build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Install python dependencies for stat_analyze.py
sudo dnf install python3.13-devel
python -m venv venv
source venv/bin/activate
pip install --upgrade pip
pip install -r requirements.txt


## Usage


### Publisher
```
./publisher <mcast_addr> <port> <msg-size> <msg-rate> <duration-sec>
```
* <duration-sec> is in seconds. The publisher will send <msg-rate> × <duration-sec> messages.

### Subscriber
```
./subscriber <mcast_addr> <port>
```

## Example
```
# Change to your build directory, e.g.:
cd mcast-1/build-opt

# Run subscriber (receives messages and writes latency stats to latency.csv)
./subscriber 239.0.0.1 5000

# Run publisher (sends messages of 100 bytes at 1 msg/sec for 10 seconds)
./publisher 239.0.0.1 5000 100 1 10
```

## Tests
./subscriber 239.0.0.1 5000
./publisher 239.0.0.1 5000 100 1 10