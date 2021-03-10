#include <cassert>
#include <netdb.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "../stl/include/io/utils.h"
#include <iostream>
#include <utility>

#include "ParsedUrl.h"
#include "Crawler.h"

// TODO: get the main() working!
// TODO: setup a Makefile (Emerson)

// TODO: redirection
// TODO: chunked result
// TODO: parse HTTP
// TODO: robots.txt parser
// TODO: setup build file for test

String FetchHtml(const ParsedUrl &url) {
    // Create and connect a socket.
    addrinfo *addr = QueryHost(url.Host, strlen(url.Port) == 0 ? url.Service : url.Port);
    int sockFd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    assert(sockFd != -1);
    int connFd = connect(sockFd, addr->ai_addr, addr->ai_addrlen);
    assert(connFd != 0);

    // Prepare a GET message
    String getRequest =
            String("GET /") + url.Path +
            " HTTP/1.1\r\n"
            "Host: " + url.Host + " \r\n" +
            "User-Agent: LinuxGetUrl/2.0 zackLt@umich.edu (Linux)\r\n"
            "Accept-Language: en-US,en;q=0.5\r\n"
            "Accept: text/html\r\n"
            "Accept-Encoding: identity\r\n"
            "Connection: close\r\n\r\n";

    String response;
    char buffer[10240];
    if (strcmp(url.Service, "http") == 0) {
        for (int sent = 0; sent < getRequest.size();) {
            int n = send(connFd, getRequest.data() + sent, getRequest.size() - sent, 0);
            assert(n > 0 && "error in send()"); // TODO: handle broken pipe
            sent += n;
        }

        int got;
        while ((got = recv(sockFd, buffer, sizeof(buffer), 0)))
            response += String(buffer, got);
    } else if (strcmp(url.Service, "https") == 0) {
        SSL_library_init();
        SSL_CTX *ctx = SSL_CTX_new(TLS_method());
        SSL *ssl = SSL_new(ctx);

        SSL_set_fd(ssl, sockFd);
        SSL_connect(ssl);

        for (int sent = 0; sent < getRequest.size();) {
            int n = SSL_write(ssl, getRequest.data() + sent, getRequest.size() - sent);
            assert(n > 0 && "error in SSL_write()"); // TODO: handle broken pipe
            sent += n;
        }

        int got;
        while ((got = SSL_read(ssl, buffer, sizeof(buffer))))
            response += String(buffer, got);

        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    } else {
        assert(false); // TODO: handle unsupported proto
    }

    freeaddrinfo(addr);

    close(sockFd);

    return response;
}

// Replying on the compiler not to reuse strings
FetchResult fetch(const String &urlString) {
}

int main(int argc, char **argv) {

    return 0;
}
