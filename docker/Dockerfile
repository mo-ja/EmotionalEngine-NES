# ベースイメージの指定
FROM ubuntu:20.04


# aptのキャッシュを更新し、必要なパッケージをインストール
RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ unzip make wget libtiff-dev \
    libopenal-dev libvorbis-dev libopusfile-dev \
    libtheora-dev libpng-dev libegl-dev libgles-dev \
    cmake tzdata patch

RUN apt-get install -y --no-install-recommends \
    git \
    && rm -rf /var/lib/apt/lists/*

# コンテナ内で実行されるコマンドを指定（例：bash）
CMD ["bash"]
