#ifndef _SERVER_H_
#define _SERVER_H_

#include <memory>
#include <functional>
#include <thread>
#include <iostream>

#include "connection.h"
#include <list>
#include "glog/logging.h"
/**
 * @brief The Server class
 */

class Server{
public:

    Server(){}

    void NewConnection();
    void DelConnection();

    int Init(int port);

    int Run();
    void Join();

    void ReceiveHandler(void *connection);
    void SendHandler(void *connection);
    void Process(void *connection);

    EpollController &GetEpollController() {return m_epoll_controller;}
    std::list<Connection> &GetConnectionPool() {return m_connection_pool;}

private:

    int m_port;
    int m_host;
    int m_socket;

    std::list<Connection> m_connection_pool;
    EpollController m_epoll_controller;
    std::thread m_io_thread;
};

int Server::Init(int port)
{
    //listen.
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(m_socket, F_SETFL, O_NONBLOCK); // set non-blocking

    int optVal = 1, ret = 0;
    if((ret = setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) < 0)) {
        LOG(ERROR) << "setsockopt error " << strerror(errno);
    }

    sockaddr_in sin;
    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);
    if(bind(m_socket, (const sockaddr*)&sin, sizeof(sin))) {
        std::cout << "bind error"  << strerror(errno) << std::endl;
        return -1;
    }

    if(listen(m_socket, 5)) {
        std::cout << "listen error" << strerror(errno) << std::endl;
        return -1;
    }

    std::cout << "server listen port: " << port << " fd: " << m_socket;
    LOG(INFO) << "server listen port: " << port << " fd: " << m_socket;

    m_epoll_controller.Init(m_socket,
                            std::bind(&Server::NewConnection, this),
                            std::bind(&Server::ReceiveHandler, this, std::placeholders::_1),
                            std::bind(&Server::SendHandler, this, std::placeholders::_1));

    m_io_thread = std::thread(std::bind(&EpollController::Run, &m_epoll_controller));
}

void Server::Join()
{
    m_io_thread.join();
    LOG(INFO) << "m_io_thread join.";
}

void Server::NewConnection()
{

    struct sockaddr_in sin;
    socklen_t len = sizeof(struct sockaddr_in);
    int n_socket, i;
    while(1) {
        //accept connections.
        if((n_socket = accept(m_socket, (struct sockaddr*)&sin, &len)) == -1)
        {
            if(errno != EAGAIN && errno != EINTR)
            {
                LOG(ERROR) << "accept error " << errno;
                return;
            }else {
                break;
            }
        }

        fcntl(n_socket, F_SETFL, O_NONBLOCK); // set non-blocking

        Connection new_connection(n_socket,
                                  m_epoll_controller,
                                  m_connection_pool);
        m_connection_pool.push_front(new_connection);

        std::list<Connection>::iterator const_iter = m_connection_pool.begin();
        const_iter->SetListIterator(const_iter);

        m_epoll_controller.AddEvent(n_socket, &(*const_iter), EPOLLIN);

        LOG(INFO) << "[new connection] "
                  << " addr: " << inet_ntoa(sin.sin_addr)
                  << " port: "<< ntohs(sin.sin_port);
    }
}

void Server::ReceiveHandler(void *connection)
{
    Connection *cur_connection = (Connection *)connection;
    if(0 != cur_connection->RecvData()) {
        LOG(ERROR) << "[receive]"
                   << " fd " << cur_connection->Socket();

        cur_connection->Destroy();
        return;
    }

    Process(connection);
}

void Server::Process(void *connection)
{
    //do some process.

    SendHandler(connection);
}

void Server::SendHandler(void *connection)
{
    Connection *cur_connection = (Connection *)connection;
    if(0 != cur_connection->SendData()) {
        LOG(ERROR) << "[send]"
                   << " fd " << cur_connection->Socket();

        cur_connection->Destroy();
        return;
    }
}

#endif
