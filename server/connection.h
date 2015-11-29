#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <memory>
#include <vector>
#include <list>
#include "glog/logging.h"

#include "epoll_controller.h"

/**
 * @brief The Connection class
 */
class Connection {

public:
    Connection(int t_socket,
               EpollController &epoll_controller,
               std::list<Connection> &connection_pool):
        m_socket(t_socket),
        status(EpollController::EPOLL_INIT),
        buf_len(0),
        m_epoll_controller(epoll_controller),
        m_connection_pool(connection_pool)
    {

    }

    int Socket() {return m_socket;}
    int Status() {return status;}

    int RecvData();
    int SendData();

    void Destroy();

    void SetListIterator(std::list<Connection>::iterator &t_iterator) { m_iterator = t_iterator; }
    std::list<Connection>::iterator GetListIterator(){ return m_iterator; }

private:

    int m_socket;
    EpollController::EPOLL_CTRL_STATUS status;
    std::list<Connection>::iterator m_iterator;

    char buf[MAX_BUFFER_SIZE+1];
    int buf_len;

    EpollController &m_epoll_controller;
    std::list<Connection> &m_connection_pool;
};

/**
 * @brief Connection::RecvData
 * @return
 */
int Connection::RecvData()
{
    int nread = 0;
    while (buf_len < MAX_BUFFER_SIZE && (nread = read(m_socket, buf + buf_len, MAX_BUFFER_SIZE+1-buf_len)) > 0) {
        buf_len += nread;
    }

    //client close.
    if(nread == 0) {
        return -1;
    }

    if (buf_len > MAX_BUFFER_SIZE ||
            (nread == -1 && errno != EAGAIN)) {
        perror("read error");
        return -1;
    }

    LOG(INFO) << "[receive] "
              << " fd: " << m_socket
              << " data len: " << buf_len;

    LOG(INFO) << "[data] " << std::string(buf, buf_len);

    //m_epoll_controller.ModEvent(m_socket, this, EPOLLOUT);

    return 0;
}

/**
 * @brief Connection::SendData
 * @return
 */
int Connection::SendData()
{
    int write_len = 0, nwrite = 0;
    while (write_len < buf_len) {
        nwrite = write(m_socket, buf + write_len, buf_len - write_len);
        if (nwrite < buf_len - write_len) {
            //not writable.
            if(errno == EAGAIN) {
                m_epoll_controller.ModEvent(m_socket, this, EPOLLOUT);
                return 0;
            }

            //write error.
            if (nwrite == -1) {
                perror("write error");
                return -1;
            }
        }
        write_len += nwrite;
    }

    m_epoll_controller.ModEvent(m_socket, this, EPOLLIN);

    LOG(INFO) << "[send] "
              << " fd: " << m_socket
              << " data len: " << buf_len;

    LOG(INFO) << "[data] " << std::string(buf, buf_len);

    buf_len = 0;

    return 0;
}

void Connection::Destroy()
{

    LOG(INFO) << "[free connection] "
              << " fd " << m_socket;

    //close socket
    close(m_socket);

    //think.
    m_epoll_controller.DelEvent(m_socket, this, EPOLLOUT);

    //erase from connection pool.
    m_connection_pool.erase(m_iterator);

    LOG(INFO) << "[Connections Remain] " << m_connection_pool.size();
}

#endif
