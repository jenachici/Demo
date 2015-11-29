#ifndef _EPOLL_CONTROLLER_H_
#define _EPOLL_CONTROLLER_H_

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <memory>
#include <vector>
#include <functional>

#define MAX_EVENTS 500
#define MAX_BUFFER_SIZE 1024*1024

/**
 * @brief The EpollController class
 */
class EpollController {

public:

    enum EPOLL_CTRL_STATUS
    {
        EPOLL_INIT,
        EPOLL_TO_READ,
        EPOLL_TO_WAIT,
        EPOLL_TO_WRITE,
        EPOLL_FINISH
    };

    EpollController(){}

    int AddEvent(int fd, void *data, int events);
    int DelEvent(int fd, void *data, int events);
    int ModEvent(int fd, void *data, int events);

    int Init(int fd,
             std::function<void()> new_connection,
             std::function<void(void *)> onread_handler,
             std::function<void(void *)> onwrite_handler);

    void Run();

private:
    int m_epoll_fd;
    int m_listen_fd;
    epoll_event m_event[MAX_EVENTS];
    std::function<void()> f_new_connection;
    std::function<void(void *)> f_onread_handler;
    std::function<void(void *)> f_onwrite_handler;
};

/**
 * @brief EpollController::Init
 * @param fd
 * @return
 */
int EpollController::Init(int fd,
                          std::function<void()> new_connection,
                          std::function<void(void *)> onread_handler,
                          std::function<void(void *)> onwrite_handler)
{
    m_epoll_fd = epoll_create(MAX_EVENTS);

    m_listen_fd = fd;

    struct epoll_event epv = {0, {0}};
    epv.events = EPOLLIN;
    epv.data.ptr = &m_listen_fd;

    if(epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &epv) < 0) {
        printf("Event Add failed[fd=%d], events[%d]\n", fd);
        return -1;
    }

    f_new_connection = new_connection;
    f_onread_handler = onread_handler;
    f_onwrite_handler = onwrite_handler;

    return 0;
}

/**
 * @brief EpollController::AddEvent
 * @param fd
 * @param data
 * @param events
 * @return
 */
int EpollController::AddEvent(int fd, void *data, int events)
{
    struct epoll_event epv = {0, {0}};
    epv.events = events;
    epv.data.ptr = data;

    if(epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &epv) < 0) {
        printf("Event Add failed[fd=%d], events[%d]\n", fd, events);
        return -1;
    }

    return 0;
}

/**
 * @brief EpollController::ModEvent
 * @param fd
 * @param data
 * @param events
 * @return
 */
int EpollController::ModEvent(int fd, void *data, int events)
{
    struct epoll_event epv = {0, {0}};
    epv.events = events;
    epv.data.ptr = data;

    if(epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &epv) < 0) {
        printf("Event Mod failed[fd=%d], events[%d]\n", fd, events);
        return -1;
    }

    return 0;
}

/**
 * @brief EpollController::DelEvent
 * @param fd
 * @param data
 * @param events
 * @return
 */
int EpollController::DelEvent(int fd, void *data, int events)
{
    struct epoll_event epv = {0, {0}};
    epv.events = events;
    epv.data.ptr = data;

    if(epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, &epv) < 0) {
        printf("Event Del failed[fd=%d], events[%d]\n", fd, events);
        return -1;
    }

    return 0;
}

/**
 * @brief EpollController::Run
 */
void EpollController::Run()
{
    while(1){
        // wait for events to happen
        int fds = epoll_wait(m_epoll_fd, m_event, MAX_EVENTS, 1000);
        if(fds < 0){
            printf("epoll_wait error, exit\n");
            break;
        }
        for(int i = 0; i < fds; i++){
            if(&m_listen_fd == m_event[i].data.ptr)
            {
                this->f_new_connection();
                continue;
            }
            if(m_event[i].events&EPOLLIN) // read event
            {
                this->f_onread_handler(m_event[i].data.ptr);
                continue;
            }
            if(m_event[i].events&EPOLLOUT) // write event
            {
                this->f_onwrite_handler(m_event[i].data.ptr);
                continue;
            }
        }
    }
}


#endif
