#include "IrcClient.hpp"

class ICommandChannel
{
public:
    virtual ~ICommandChannel() { }

    virtual void Send(std::string message) = 0;
    virtual std::string Receive() = 0;
    std::string GetName() { return nick_; }

protected:
    std::string nick_;
};

class IrcCommandChannel : public ICommandChannel
{
public:
    IrcCommandChannel(std::string host, std::string port, std::string channel, std::string herder, std::string pem);
    virtual ~IrcCommandChannel();

    virtual void Send(std::string message) override;
    virtual std::string Receive() override;

private:
    std::unique_ptr<Legit::IrcClient> client_;
    std::string channel_;
    std::string herder_;
};

class CustomCommandChannel : public ICommandChannel
{
public:
    CustomCommandChannel(std::string host, std::string port, std::string pem);
    virtual ~CustomCommandChannel();

    virtual void Send(std::string message) override;
    virtual std::string Receive() override;

public:
    std::unique_ptr<Legit::ISocket> socket_;
    std::string remains_;
};

