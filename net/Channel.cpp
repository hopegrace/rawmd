/******************************************************************************************
 * Read And Write Muduo(RAWMD)
 * wzhe, ahuwang@163.com
 ******************************************************************************************/

#include <base/Logging.h>
#include <net/Channel.h>
#include <net/EventLoop.h>

#include <sstream>
#include <poll.h>

using namespace rawmd;
using namespace rawmd::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd__)
    : loop_(loop),
      fd_(fd__),
      events_(0),
      revents_(0),
      index_(-1),
      logHup_(true),
      tied_(false),
      eventHandling_(false)
{
}

Channel::~Channel()
{
    assert(!eventHandling_);
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    loop_->updateChannel(this);
}

// 调用这个函数之前确保调用disableALL
void Channel::remove()
{
    assert(isNoneEvent());
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    std::shared_ptr<void> guard;
    if (tied_)
    {
        guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    eventHandling_ = true;
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
    if (logHup_)
    {
      LOG_WARN << "Channel::handle_event() POLLHUP";
    }
    if (closeCallback_) closeCallback_();
    }

    if (revents_ & POLLNVAL)
    {
    LOG_WARN << "Channel::handle_event() POLLNVAL";
    }

    if (revents_ & (POLLERR | POLLNVAL))
    {
    if (errorCallback_) errorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
    if (readCallback_) readCallback_(receiveTime);
    }
    if (revents_ & POLLOUT)
    {
    if (writeCallback_) writeCallback_();
    }
    eventHandling_ = false;
}

string Channel::reventsToString() const
{
    std::ostringstream oss;
    oss << fd_ << ": ";
    if (revents_ & POLLIN)
    oss << "IN ";
    if (revents_ & POLLPRI)
    oss << "PRI ";
    if (revents_ & POLLOUT)
    oss << "OUT ";
    if (revents_ & POLLHUP)
    oss << "HUP ";
    if (revents_ & POLLRDHUP)
    oss << "RDHUP ";
    if (revents_ & POLLERR)
    oss << "ERR ";
    if (revents_ & POLLNVAL)
    oss << "NVAL ";

    return oss.str().c_str();
}