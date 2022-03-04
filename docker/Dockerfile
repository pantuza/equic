FROM ubuntu:latest

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=America/Sao_Paulo

RUN apt-get update && \
    apt-get install -y build-essential git cmake \
                       software-properties-common \
                       zlib1g-dev libevent-dev golang \
                       llvm vim libelf-dev iputils-ping \
                       clang netcat tcpdump dnsutils iproute2 \
                       libc6-dev-i386


RUN mkdir /src
WORKDIR /src


RUN git clone https://boringssl.googlesource.com/boringssl && \
    cd boringssl && \
    git checkout 251b5169fd44345f455438312ec4e18ae07fd58c && \
    cmake . && \
    make

RUN git clone https://github.com/litespeedtech/lsquic.git && \
    cd lsquic && \
    git submodule update --init --recursive

RUN cd /src/lsquic && \
    cmake -DBORINGSSL_DIR=/src/boringssl . && \
    make

RUN cd lsquic && make test && cp bin/http_client /usr/bin/ && cp bin/http_server /usr/bin

#
# Configures eQUIC
#
COPY . /src/equic

WORKDIR /src/equic

# TODO: Check how we can do it cause docker build does not accept
# Mounts kernel tracing debug filesystem for debugging eBPF
# mount -t debugfs none /sys/kernel/debug && \

# Needed to link asm/byteorder.h into eBPF
RUN ln -s /usr/include/x86_64-linux-gnu/asm/ /usr/include/asm


# Clone and build libbpf as staticlibrary
RUN git submodule update --init && \
    cd libbpf/src && \
    make BUILD_STATIC_ONLY=y && \
    make install BUILD_STATIC_ONLY=y LIBDIR=/usr/lib/x86_64-linux-gnu/

# Clones the master branch of linux kernel
RUN git clone --depth 1 git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git && \
    # Overrides all bpf headers with the latest ones from linux kernel source
    cp linux/include/uapi/linux/bpf* /usr/include/linux/
    # TODO: Remove linux folder after copying bpf files to release space
