/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "tcpserver.h"

namespace debugger {

TcpServer::TcpServer(const char *name) : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("Timeout", &timeout_);
    registerAttribute("BlockingMode", &blockmode_);
    registerAttribute("HostIP", &hostIP_);
    registerAttribute("HostPort", &hostPort_);
    registerAttribute("PlatformConfig", &platformConfig_);
    registerAttribute("Type", &type_);
    registerAttribute("ListenDefaultOutput", &listenDefaultOutput_);
    registerAttribute("JtagTap", &jtagtap_);
}

void TcpServer::postinitService() {
    createServerSocket();

    if (listen(hsock_, 1) < 0)  {
        RISCV_error("listen() failed", 0);
        return;
    }

    /** By default socket was created with Blocking mode */
    if (!blockmode_.to_bool()) {
        setBlockingMode(false);
    }

    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

void TcpServer::busyLoop() {
    socket_def client_sock;
    int err;

    fd_set readSet;
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 400000;   // 400 ms

    int idx = 0;
    char tname[64];

    IClass *icls;
    IService *isrv;
    while (isEnabled()) {
        FD_ZERO(&readSet);
        FD_SET(hsock_, &readSet);
        err = select(hsock_ + 1, &readSet, NULL, NULL, &timeout);
        if (err > 0) {
            client_sock = accept(hsock_, 0, 0);
            setRcvTimeout(client_sock, timeout_.to_int());
            RISCV_sprintf(tname, sizeof(tname), "client%d", idx++);

            AttributeType lst, item;
            lst.make_list(0);
            item.make_list(2);
            if (type_.is_equal("openocd")) {
                icls = static_cast<IClass *>(RISCV_get_class("TcpJtagBitBangClientClass"));
                isrv = icls->createService(".", tname);
                item[0u].make_string("LogLevel");
                item[1].make_int64(logLevel_.to_int());
                lst.add_to_list(&item);
                item[0u].make_string("Enable");
                item[1].make_boolean(true);
                lst.add_to_list(&item);
                item[0u].make_string("JtagTap");
                item[1].clone(&jtagtap_);
                lst.add_to_list(&item);
            } else {
                icls = static_cast<IClass *>(RISCV_get_class("TcpClientClass"));
                isrv = icls->createService(".", tname);
                item[0u].make_string("LogLevel");
                item[1].make_int64(logLevel_.to_int());
                lst.add_to_list(&item);
                item[0u].make_string("Enable");
                item[1].make_boolean(true);
                lst.add_to_list(&item);
                item[0u].make_string("PlatformConfig");
                item[1].clone(&platformConfig_);
                lst.add_to_list(&item);
                item[0u].make_string("Type");
                item[1].clone(&type_);
                lst.add_to_list(&item);
                item[0u].make_string("ListenDefaultOutput");
                item[1].clone(&listenDefaultOutput_);
                lst.add_to_list(&item);
            }

            isrv->initService(&lst);
            IThread *ithrd =
                static_cast<IThread *>(isrv->getInterface(IFACE_THREAD));
            ithrd->setExtArgument(&client_sock);
            isrv->postinitService();
            RISCV_info("TCP %s %p started", isrv->getObjName(), client_sock);
        } else if (err == 0) {
            // timeout
        } else {
            RISCV_info("TCP server thread accept() failed", 0);
            loopEnable_.state = false;
        }
    }
    closeServerSocket();
}

int TcpServer::createServerSocket() {
    char hostName[256];
    if (gethostname(hostName, sizeof(hostName)) < 0) {
        return -1;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    /**
     * Check availability of IPv4 address assigned via attribute 'hostIP'.
     * If it woudn't be found use the last avaialble IP address.
     */
    int retval;
    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    retval = getaddrinfo(hostName, "0", &hints, &result);
    if (retval != 0) {
        return -1;
    }

    if (hostIP_.size() == 0 || hostIP_.is_equal("127.0.0.1")) {
        memset(&sockaddr_ipv4_, 0, sizeof(struct sockaddr_in));
        sockaddr_ipv4_.sin_family = AF_INET;
        sockaddr_ipv4_.sin_addr.s_addr = inet_addr("127.0.0.1");
    } else {
        for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
            // Find only IPV4 address, ignore others.
            if (ptr->ai_family != AF_INET) {
                continue;
            }
            sockaddr_ipv4_ = *((struct sockaddr_in *)ptr->ai_addr);

            if (hostIP_.is_equal(inet_ntoa(sockaddr_ipv4_.sin_addr))) {
                break;
            }
        }
    }
    sockaddr_ipv4_.sin_port = htons(static_cast<uint16_t>(hostPort_.to_int()));
    RISCV_info("Selected Host IPv4 %s:%d",
                inet_ntoa(sockaddr_ipv4_.sin_addr),
                hostPort_.to_uint32());

    hsock_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (hsock_ < 0) {
        RISCV_error("%s", "Error: socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)");
        return -1;
    }

    int enable = 1;
    if (setsockopt(hsock_, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char *>(&enable), sizeof(int)) < 0) {
        RISCV_error("%s", "setsockopt(SO_REUSEADDR) failed");
        return -1;
    }

    int res = bind(hsock_,
                   reinterpret_cast<struct sockaddr *>(&sockaddr_ipv4_),
                   sizeof(sockaddr_ipv4_));
    if (res != 0) {
        RISCV_error("Error: bind(hsock_, \"%s\", ...)", hostIP_.to_string());
        return -1;
    }

    addr_size_t addr_sz = sizeof(sockaddr_ipv4_);
    res = getsockname(hsock_,
                      reinterpret_cast<struct sockaddr *>(&sockaddr_ipv4_),
                      &addr_sz);

    RISCV_info("IPv4 address %s:%d . . . opened",
                inet_ntoa(sockaddr_ipv4_.sin_addr),
                ntohs(sockaddr_ipv4_.sin_port));

    return 0;
}

void TcpServer::setRcvTimeout(socket_def skt, int timeout_ms) {
    if (!timeout_ms) {
        return;
    }
    struct timeval tv;
#if defined(_WIN32) || defined(__CYGWIN__)
    /** On windows timeout of the setsockopt() function is the DWORD
        * size variable in msec, so we use only the first field in timeval
        * struct and directly assgign argument.
        */
    tv.tv_sec = timeout_ms;
    tv.tv_usec = 0;
#else
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    tv.tv_sec = timeout_ms / 1000;
#endif
    setsockopt(skt, SOL_SOCKET, SO_RCVTIMEO,
                    reinterpret_cast<char *>(&tv), sizeof(struct timeval));
}

bool TcpServer::setBlockingMode(bool mode) {
    int ret;
#if defined(_WIN32) || defined(__CYGWIN__)
    // 0 = disable non-blocking mode
    // 1 = enable non-blocking mode
    u_long arg = mode ? 0 : 1;
    ret = ioctlsocket(hsock_, FIONBIO, &arg);
    if (ret == SOCKET_ERROR) {
        RISCV_error("Set non-blocking socket failed", 0);
    }
#else
    int flags = fcntl(hsock_, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    flags = mode ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    ret = fcntl(hsock_, F_SETFL, flags);
#endif
    if (ret == 0) {
        // success
        blockmode_.make_boolean(mode);
        return true;
    }
    return false;
}

void TcpServer::closeServerSocket() {
    if (hsock_ < 0) {
        return;
    }

#if defined(_WIN32) || defined(__CYGWIN__)
    closesocket(hsock_);
#else
    shutdown(hsock_, SHUT_RDWR);
    close(hsock_);
#endif
    hsock_ = -1;
}


}  // namespace debugger
