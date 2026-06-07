FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Dependências do sistema + Qt6
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    libqt6widgets6 \
    libgl1-mesa-dev \
    libx11-dev \
    libxcb-xinerama0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-randr0 \
    libxcb-render-util0 \
    libxcb-shape0 \
    libxcb-xkb1 \
    libxkbcommon-x11-0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copia todo o projeto
COPY . .

# Build da UI
RUN cmake -S ui -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --parallel

CMD ["./build/ui"]
