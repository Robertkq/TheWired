#ifndef kqTheWiredServer
#define kqTheWiredServer

#include <kqNet.h>
#include "..\TheWiredCommon\common.h"

struct user
{

    bool operator==(kq::connection<msgids>* other) const { return remote == other; }
    bool operator==(const std::string& _name) const { return name == _name; }

    kq::connection<msgids>* remote;
    std::string name;
};

struct server : kq::server_interface<msgids>
{
    server(uint16_t port, uint64_t(*scrambleFunc)(uint64_t)) : kq::server_interface<msgids>(port, scrambleFunc) {}

    bool OnClientConnect(kq::connection<msgids>* client) override
    {
        color(8);
        std::cout << "New connection [ " << client->getIP() << " ], validating...\n";
        color(7);
        return true;
    }
    void OnClientDisconnect(kq::connection<msgids>* client) override
    {
        color(4);
        std::cout << "Connection [ " << client->getIP() << " ], disconnected!\n";
        color(7);
        auto it = std::find(users.begin(), users.end(), client);
        if (it != users.end())
        {
            std::cout << it->name << " removed from users.\n";
            users.erase(it);
        }
        std::cout << "Users left:\n";
        for (auto user : users)
        {
            std::cout << user.name << "\n";
        }
    }
    void OnClientValidated(kq::connection<msgids>* client) override
    {
        color(2);
        std::cout << "Connection [ " << client->getIP() << " ], validated!\n";
        color(7);
    }
    void OnClientUnvalidated(kq::connection<msgids>* client) override
    {
        color(4);
        std::cout << "Connection [ " << client->getIP() << " ], unvalidated!\n";
        color(7);
    }
    void OnMessage(kq::connection<msgids>* client, kq::message<msgids>& msg) override
    {
        switch (msg.getID())
        {
        case(msgids::ServerAcceptName):
            break;
        case(msgids::ServerRejectName):
            break;
        case(msgids::ClientProvideName):
        {
            uint32_t size;
            std::string name;
            msg >> size;
            name.resize(size);
            for (int i = 0; i < size; ++i)
                msg >> name[i];
            std::reverse(name.begin(), name.end());
            kq::message<msgids> answer;
            if (nameExists(name))
            {
                answer.getID() = msgids::ServerRejectName;
            }
            else
            {
                answer.getID() = msgids::ServerAcceptName;
                users.push_back({ client, name });
            }
            MessageClient(client, answer);
            break;
        }
        case(msgids::Whisper):
        {
            std::string whisperName, whisperMessage;
            uint32_t whisperNameSize, whisperMessageSize;
            msg >> whisperNameSize;
            whisperName.resize(whisperNameSize);
            for (int i = 0; i < whisperNameSize; ++i)
                msg >> whisperName[i];
            msg >> whisperMessageSize;
            whisperMessage.resize(whisperMessageSize);
            for (int i = 0; i < whisperMessageSize; ++i)
                msg >> whisperMessage[i];

            std::reverse(whisperName.begin(), whisperName.end());
            std::reverse(whisperMessage.begin(), whisperMessage.end());

            auto it = std::find(users.begin(), users.end(), whisperName);
            auto itc = std::find(users.begin(), users.end(), client);
            if (it != users.end())
            {
               //std::cout << "A whisper to: " << whisperName << " from " << it->name << ", containing: " << whisperMessage << '\n';
                kq::message<msgids> answer(msgids::Whisper);
                for (int i = 0; i < whisperMessageSize; ++i)
                    answer << static_cast<char>(whisperMessage[i]);
                answer << static_cast<uint32_t>(whisperMessageSize);
                for (int i = 0; i < itc->name.size(); ++i)
                    answer << static_cast<char>(itc->name[i]);
                answer << static_cast<uint32_t>(itc->name.size());

                //std::cout << "Comparasion before sending: " << client->getIP() << " = " << it->remote->getIP() << '\n';

                MessageClient(it->remote, answer);
            }
                
            break;
        }
        case(msgids::MessageAll):
        {
            auto it = std::find(users.begin(), users.end(), client);
            if (it != users.end())
            {
                for (int i = 0; i < it->name.size(); ++i)
                    msg << static_cast<char>(it->name[i]);
                msg << static_cast<uint32_t>(it->name.size());
            }
            MessageAllClients(client, msg);
            break;
        }
            
        case(msgids::Confetti):
            auto it = std::find(users.begin(), users.end(), client);
            
            if (it != users.end())
            {
                for (int i = 0; i < it->name.size(); ++i)
                    msg << static_cast<char>(it->name[i]);
                msg << static_cast<uint32_t>(it->name.size());
            }
            MessageAllClients(client, msg);
            break;
        }
    }

    bool nameExists(const std::string& name) const
    {
        for (auto user : users)
        {
            if (name == user.name)
                return true;
        }
        return false;
    }

    std::vector<user> users;

};

#endif