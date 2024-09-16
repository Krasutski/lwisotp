FROM python:3.11

ENV REFRESHED_AT 2024-09-16

# Copy all necessary files
COPY ./.devcontainer/bashrc.template /root/.bashrc

RUN apt-get update \
    && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends software-properties-common \
    && add-apt-repository "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-18 main" \
    && apt-get -y install \
    git \
    bzr \
    lib32z1 \
    cmake \
    ninja-build \
    gdb \
    protobuf-compiler \
    automake \
    autoconf \
    usbutils \
    make \
    libncurses5 \
    build-essential \
    libtool \
    gcc-multilib \
    g++-multilib \
    bc \
    lcov \
    unzip \
    device-tree-compiler \
    tcl-dev \
    ccache \
    lsb-release \
    wget \
    software-properties-common \
    gnupg


# Update PIP
RUN pip3 install --no-cache-dir --upgrade pip
RUN pip3 install commitizen pycryptodome kconfiglib pre-commit pyelftools python-gitlab intelhex


ENV CLANG_VERSION="19"
RUN wget https://apt.llvm.org/llvm.sh && chmod +x llvm.sh && ./llvm.sh ${CLANG_VERSION} \
    && apt-get -y install clang-format-${CLANG_VERSION} clang-tidy-${CLANG_VERSION}
# Setup symlinks for clang tools
RUN ln -s /usr/bin/clang-tidy-${CLANG_VERSION} /usr/bin/clang-tidy \
    && ln -s /usr/bin/clang-format-${CLANG_VERSION} /usr/bin/clang-format


#CppUTest setup
RUN git clone https://github.com/cpputest/cpputest.git /opt/cpputest -b v4.0 --recursive --shallow-submodules --depth 1 \
    && cd /opt/cpputest \
    && autoreconf . -i \
    && ./configure "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32" \
    && make tdd -j$(nproc) \
    && find . -maxdepth 1 ! \( -name '.' -o -name 'lib' -o -name 'include' -o -name 'scripts' \) -exec rm -rf {} +
ENV CPPUTEST_HOME="/opt/cpputest" \
    PATH="/opt/cpputest/scripts:${PATH}"

#CppUMockGen setup
# RUN mkdir -p /opt/cppumockgen \
#     && wget -P /opt/cppumockgen/ https://github.com/jgonzalezdr/CppUMockGen/releases/download/v0.4/cppumockgen_0.4-22_amd64.deb \
#     && apt-get update \
#     && apt-get install /opt/cppumockgen/cppumockgen_0.4-22_amd64.deb -y \
#     && rm -rf /var/lib/apt/lists/* \
#     && rm -rf /opt/cppumockgen

# Set up a tools dev directory
WORKDIR /usr

# Download ARM GNU toolchain
ENV GCC_ARM_VERSION="arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi"
RUN wget -q --show-progress --progress=bar:force:noscroll --no-check-certificate --output-document=arm-none-eabi.tar.xz https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz
RUN tar -xf arm-none-eabi.tar.xz
RUN rm arm-none-eabi.tar.xz

ENV GCC_PATH /usr/${GCC_ARM_VERSION}
ENV PATH /usr/${GCC_ARM_VERSION}/bin:$PATH

RUN rm -rf /var/lib/apt/lists/*
