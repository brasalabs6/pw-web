# Cloudflare Containers runtime image for the PW 1.5.3 server.
# linux/amd64 with i386 multilib: the server binaries are 32-bit (g++ -m32).
# Build context must contain: server153/ (daemon tree staged by
# scripts/cloudflare-server153.sh) and entrypoint.sh.
#
# pw-web note: busybox (NOT python3) serves the health endpoint. A python3
# http.server entrypoint is consistently refused by the platform ("no
# container instance can be provided" — bisected with probe apps: busybox
# entrypoints provision at any image size; python3 ones never do).
FROM debian:12

RUN dpkg --add-architecture i386 && apt-get update && \
    apt-get install -y --no-install-recommends \
      libc6:i386 libgcc-s1:i386 libstdc++6:i386 libssl3:i386 libpcre3:i386 \
      libxml2:i386 busybox ca-certificates && \
    rm -rf /var/lib/apt/lists/*

COPY server153 /opt/server153
COPY entrypoint.sh /opt/
RUN strip /opt/server153/cnet/*/gauthd /opt/server153/cnet/*/gdeliveryd \
      /opt/server153/cnet/*/gamedbd /opt/server153/cnet/*/uniquenamed \
      /opt/server153/cnet/*/gfactiond /opt/server153/cnet/*/gacd \
      /opt/server153/cnet/*/logservice /opt/server153/cnet/*/glinkd \
      /opt/server153/cgame/gs/gs 2>/dev/null || true; \
    chmod +x /opt/entrypoint.sh

EXPOSE 8080 29000
ENTRYPOINT ["/opt/entrypoint.sh"]
