# Compilador do Nuvio .tpk (Samsung Tizen 6+), ARM softfp.
#
# Debian buster armel: glibc 2.28, abaixo da 2.30 do Tizen 6.0, entao o que sai
# daqui carrega em toda TV alvo. armel = mesma ABI de chamada do softfp da
# Samsung; o -march/-mfpu nas CFLAGS e que liga NEON/VFP.
# O gcc-14.2 do Tizen Studio no Mac roda por Rosetta e da "internal compiler
# error: Illegal instruction" ao compilar o SDL — por isso o build e aqui.
FROM --platform=linux/arm/v5 arm32v5/debian:buster-slim
RUN sed -i 's#deb.debian.org#archive.debian.org#g; /security/d' /etc/apt/sources.list && \
    apt-get -qq update && apt-get -qq install -y --no-install-recommends \
      gcc g++ make pkg-config file libgles2-mesa-dev libegl1-mesa-dev zlib1g-dev \
      ca-certificates && rm -rf /var/lib/apt/lists/*
ENV CFLAGS="-O2 -fPIC -march=armv7-a -mfloat-abi=softfp -mfpu=neon" \
    CXXFLAGS="-O2 -fPIC -march=armv7-a -mfloat-abi=softfp -mfpu=neon"
