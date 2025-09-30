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
./publisher <core> <sender_id> <mcast_addr> <port> <msg-size> <msg-rate> <duration-sec>
```
* `<core>` is the CPU core to pin the publisher thread to (int64_t)
* `<sender_id>` is a unique 64-bit integer identifier for the publisher
* `<duration-sec>` is in seconds. The publisher will send `<msg-rate>` × `<duration-sec>` messages.


### Subscriber
```
./subscriber <core> <mcast_addr> <port>
```
* `<core>` is the CPU core to pin the subscriber thread to (int64_t)

### Sequencer
```
./sequencer <core> <recv_mcast_addr> <recv_port> <send_mcast_addr> <send_port>
```
* `<core>` is the CPU core to pin the sequencer thread to (int64_t)
The sequencer listens on the `recv_mcast_addr:recv_port`, timestamps and tags messages, then forwards them to `send_mcast_addr:send_port`.

## Example
```
# Change to your build directory, e.g.:
cd mcast-1/build-opt

# Run subscriber (core=0, receives messages and writes latency stats to latency.csv)
./subscriber 0 239.0.0.1 5000


# Run publisher (sender_id=1, core=0, sends messages of 100 bytes at 1 msg/sec for 10 seconds)
./publisher 2 1 239.0.0.1 5000 100 1 10

# Example using a sequencer between publisher and subscriber
# Publisher -> Sequencer -> Subscriber
# Publisher sends to recv group 239.0.0.1:5000
# Sequencer listens on 239.0.0.1:5000 and forwards to 239.0.0.2:6000
# Subscriber listens on 239.0.0.2:6000


./subscriber 0 239.0.0.2 6000
./sequencer 1 239.0.0.1 5000 239.0.0.2 6000

./publisher 2 1 239.0.0.1 5000 100 1 10
```

then run the analyzer (within the venv):
../analyze_stats.py


## temp
cd build-opt
source ../../venv/bin/activate

./subscriber 1 239.0.0.2 6000 && ../analyze_stats.py

./sequencer 2 239.0.0.1 5000 239.0.0.2 6000

./publisher 3 1 239.0.0.1 5000 100 1 10

