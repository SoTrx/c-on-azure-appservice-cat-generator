# Dockerfile for a raw minimal container (Expected size 250kB)
####################################################################################################
## Builder
####################################################################################################
FROM gcc as builder
WORKDIR /app
COPY . .
RUN DEBIAN_FRONTEND=noninteractive apt update && DEBIAN_FRONTEND=noninteractive apt install -y cmake upx
# Build the app, strip it (LDFLAGS) and optimize it with UPX
RUN cmake -DCMAKE_BUILD_TYPE=Release . &&\
    make &&\
    strip c_on_azure_cat_api &&\
    upx --best --lzma c_on_azure_cat_api


