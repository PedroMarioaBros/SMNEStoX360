FROM ghcr.io/cirruslabs/android-sdk:35 AS builder

WORKDIR /src
COPY . .

RUN wget -q https://services.gradle.org/distributions/gradle-8.9-bin.zip -O /tmp/gradle.zip \
    && unzip -q /tmp/gradle.zip -d /opt \
    && rm /tmp/gradle.zip

RUN /opt/gradle-8.9/bin/gradle :app:assembleDebug --no-daemon --stacktrace

FROM nginx:alpine
COPY --from=builder /src/app/build/outputs/apk/debug/app-debug.apk /usr/share/nginx/html/Extrator-Transcricao-v1.apk
RUN printf '<!doctype html><html><head><meta charset="utf-8"><title>Extrator de Transcrição</title></head><body><h1>Extrator de Transcrição v1</h1><p><a href="/Extrator-Transcricao-v1.apk">Baixar APK</a></p></body></html>' > /usr/share/nginx/html/index.html
EXPOSE 80
