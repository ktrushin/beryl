ARG base
FROM ${base}

RUN apt-get update && apt-get install --yes --no-install-recommends \
        iproute2 bind9 bind9-dnsutils tcpdump
