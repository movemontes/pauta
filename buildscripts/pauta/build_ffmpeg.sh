#!/usr/bin/env bash
# Pauta: builds the small LGPL FFmpeg that ships inside Pauta.app (score video export).
# Only what the video needs: MP4 in and out, the Mac's own H.264 encoder (VideoToolbox), AAC copied as is.
# Everything stays on the Dados disk. Usage: bash buildscripts/pauta/build_ffmpeg.sh
set -euo pipefail
ROOT=/Volumes/Dados/PautaCodigo
VER=7.1.5
cd "$ROOT/ffmpeg"
[ -f "ffmpeg-$VER.tar.xz" ] || curl -sSLO "https://ffmpeg.org/releases/ffmpeg-$VER.tar.xz"
[ -d "ffmpeg-$VER" ] || tar xf "ffmpeg-$VER.tar.xz"
cd "ffmpeg-$VER"
./configure --prefix="$ROOT/ffmpeg/install" \
    --enable-shared --disable-static --disable-programs --disable-doc --disable-debug \
    --disable-everything --disable-autodetect --disable-network \
    --enable-videotoolbox --enable-pthreads \
    --enable-encoder=h264_videotoolbox --enable-decoder=h264,aac \
    --enable-parser=h264,aac --enable-demuxer=mov,aac --enable-muxer=mp4,mov \
    --enable-protocol=file --enable-bsf=aac_adtstoasc \
    --enable-swscale --enable-swresample --enable-avformat --enable-avcodec \
    --install-name-dir='@loader_path' --arch=arm64 \
    --extra-cflags='-mmacosx-version-min=12.0' --extra-ldflags='-mmacosx-version-min=12.0'
make -j3
make install
