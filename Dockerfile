# Use Ubuntu as base image
FROM ubuntu:20.04

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source files
COPY . .

# Build the project
RUN mkdir build && cd build && cmake .. && make

# Default command
CMD ["./build/app"]