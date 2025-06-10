FROM ubuntu:22.04

# Basic Setup
RUN apt-get update && apt-get install -y \
    build-essential \
    python3 \
    python3-pip \
    time \
    util-linux \
    && rm -rf /var/lib/apt/lists/*

RUN apt update && apt install openjdk-21-jdk -y

# INSTALL YOUR SOLVER AND DEPENDENCIES HERE
COPY pace-eval-ds-exact/solver/pace2025/ /build
WORKDIR /build

RUN apt-get install -y \
    cmake \ 
    libargtable2-dev \
    libomp-dev

RUN cmake -DCMAKE_BUILD_TYPE=Release . 
RUN cmake --build .
RUN mkdir -p /solver && cp main.out /solver/dshunter

# END OF YOUR CODE HERE.

# working directory
WORKDIR /pace
COPY src/eval.py eval.py
COPY src/ds_verifier.py verifier.py

RUN mkdir -p /output && chmod 777 /output
CMD ["python3", "eval.py"]
