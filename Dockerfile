FROM debian:stable-slim

RUN apt-get update && \
    apt-get install -y build-essential cmake && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /violet

COPY . .

RUN chmod +x build.sh && ./build.sh

VOLUME ["/data"]

ENV DUMP_PATH=/data/dump.dbz

EXPOSE 2005

CMD ["./build/violet"]
