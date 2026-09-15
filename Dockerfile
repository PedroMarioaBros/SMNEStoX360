FROM ubuntu:24.04 AS build
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates git cmake ninja-build build-essential && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /src
ARG XEXTOOL_COMMIT=cca82a33b3a14de00042f62ba6b009b9f92c4c39
RUN git clone --no-checkout https://github.com/Team-Resurgent/XexTool.git XexTool && \
    cd XexTool && \
    git fetch --depth=1 origin "$XEXTOOL_COMMIT" && \
    git checkout --detach FETCH_HEAD && \
    git submodule sync --recursive && \
    git submodule update --init --recursive --depth=1 && \
    test "$(git rev-parse HEAD)" = "$XEXTOOL_COMMIT"
RUN cmake -S /src/XexTool -B /src/XexTool/build -G Ninja -DCMAKE_BUILD_TYPE=Release && \
    cmake --build /src/XexTool/build --parallel 2 && \
    ctest --test-dir /src/XexTool/build --output-on-failure && \
    sha256sum /src/XexTool/build/XexTool > /src/XexTool/build/XexTool.sha256
RUN printf 'source=https://github.com/Team-Resurgent/XexTool\ncommit=%s\n' "$XEXTOOL_COMMIT" > /src/XexTool/build/XexTool.BUILD.txt

FROM python:3.12-slim
WORKDIR /srv
COPY --from=build /src/XexTool/build/XexTool /srv/XexTool
COPY --from=build /src/XexTool/build/XexTool.sha256 /srv/XexTool.sha256
COPY --from=build /src/XexTool/build/XexTool.BUILD.txt /srv/XexTool.BUILD.txt
RUN chmod +x /srv/XexTool
CMD ["sh", "-c", "python -m http.server ${PORT:-8080} --bind 0.0.0.0 --directory /srv"]
