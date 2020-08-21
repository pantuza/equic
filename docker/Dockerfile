FROM ubuntu:latest

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=America/Sao_Paulo

RUN apt-get update -y && \
    # Project dependencies
    apt-get install -y iproute2 tcpdump git sudo dnsutils \
                       vim clang llvm libelf-dev && \
    # mvfst dependencies
    apt-get install -y g++ cmake libboost-all-dev libevent-dev \
                    libdouble-conversion-dev libgoogle-glog-dev libgflags-dev \
                    libiberty-dev liblz4-dev liblzma-dev libsnappy-dev make \
                    zlib1g-dev binutils-dev libjemalloc-dev libssl-dev \
                    pkg-config  libsodium-dev


# Builds Facebook mvfst project
RUN git clone https://github.com/facebookincubator/mvfst.git && \
    cd mvfst && ./build_helper.sh


#
# Configures eQUIC
#
COPY . /equic

WORKDIR /equic

# TODO: Check how we can do it cause docker build does not accept
# Mounts kernel tracing debug filesystem for debugging eBPF
# mount -t debugfs none /sys/kernel/debug && \

# Needed to link asm/byteorder.h into eBPF
RUN ln -s /usr/include/x86_64-linux-gnu/asm/ /usr/include/asm

# Clone and build libbpf as staticlibrary
RUN git submodule update --init && \
    cd libbpf/src && \
    make && \
    make install

# Clones the master branch of linux kernel
RUN git clone --depth 1 git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git && \
    # Overrides all bpf headers with the latest ones from linux kernel source
    cp linux/include/uapi/linux/bpf* /usr/include/linux/
    # TODO: Remove linux folder after copying bpf files to release space
