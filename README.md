TCP Proxy
---

You should change the addresses and ports in the source code (which is defined) to use it.

It listens on `LISTEN_ADDRESS`-`LISTEN_PORT` for incoming clients and
connects to `CONN_ADDRESS`-`CONN_PORT`.

#### compile 
>don't forget to configure addresses and ports

    gcc tcp_proxy.c -o tcp_proxy

#### usage:
    ./tcp_proxy
