#include "client.h"

bool EnterName();
void ChatRoom();
void ReadCommand();


client Client(&scramble);

int main()
{
    
    
    if (Client.Connect(host_ip, host_port) == true)
    {
        color(8);
        std::cout << "Welcome to ";
        color(5);
        std::cout << "The Wired!\n";
        color(7);
        while(EnterName()) {}
        // Once a user is past this while loop, they have succesfully confirmed their nickname with the server

        color(8);
        std::cout << "You're in...\n";
        color(7);

        ChatRoom();
        
    }

    return 0;
}


bool EnterName()
{
    do
    {
        std::cout << "Please enter your nickname: ";
            std::cin >> Client.name;
    } while (Client.name.size() < 3 || Client.name.size() > 15);
    
    kq::message<msgids> sendName{ msgids::ClientProvideName };
    
    for (int i = 0; i < Client.name.size(); ++i)
        sendName << static_cast<char>(Client.name[i]);
    sendName << static_cast<uint32_t>(Client.name.size());

    Client.Send(sendName);


    while (true)
    {
        if (Client.IsConnected() && !Client.Incoming().empty())
        {
            auto msg = Client.Incoming().front().msg;
            Client.Incoming().pop_front();
            switch (msg.getID())
            {
            case(msgids::ServerAcceptName):
                return false;
                break;
            case(msgids::ServerRejectName):
                std::cout << "This nickname is already taken, please try another one.\n";
                return true;
                break;
            
            }
        }
    }
}

void ChatRoom()
{
    static bool newCtrl = false;
    static bool oldCtrl = false;


    while (true)
    {
        if (GetConsoleWindow() == GetForegroundWindow())
        {
            oldCtrl = newCtrl;
            newCtrl = GetAsyncKeyState(VK_CONTROL);
        }

        if (newCtrl && !oldCtrl)
        {
            ReadCommand();
        }

        if (Client.IsConnected() && !Client.Incoming().empty())
        {
            auto msg = Client.Incoming().front().msg;
            Client.Incoming().pop_front();
            switch (msg.getID())
            {
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

                color(5);
                std::cout << "(Whisper From) " << whisperName << ": " << whisperMessage << '\n';
                Client.lastWhisper = whisperName;
                color(7);

                break;
            }
            case(msgids::MessageAll):
            {
                std::string name, message;
                uint32_t nameSize, messageSize;
                msg >> nameSize;
                name.resize(nameSize);
                for (int i = 0; i < nameSize; ++i)
                    msg >> name[i];

                msg >> messageSize;
                message.resize(messageSize);
                for (int i = 0; i < messageSize; ++i)
                    msg >> message[i];

                std::reverse(name.begin(), name.end());
                std::reverse(message.begin(), message.end());

                std::cout << name << ": " << message << '\n';
                
                break;
            }
            case(msgids::Confetti):
            {
                std::string name;
                uint32_t nameSize;
                msg >> nameSize;
                name.resize(nameSize);
                for (int i = 0; i < nameSize; ++i)
                    msg >> name[i];
                std::reverse(name.begin(), name.end());
                
                std::cout << name;
                color(2);
                std::cout << " threw ";
                color(5);
                std::cout << "confetti";
                color(9);
                std::cout << " all ";
                color(1);
                std::cout << "over";
                color(3);
                std::cout << " the ";
                color(4);
                std::cout << "chat";
                color(2);
                std::cout << " room!\n";
                color(7);

                break;
            }
            }
        }
    }

}

void ReadCommand()
{
    std::string command;
    std::cin.ignore(int64_t{ -1 }, '\n');
    std::getline(std::cin, command);

    kq::message<msgids> answer;

    if (command[0] != '/')
    {
        // It's just a normal message to all the clients connected
        answer.getID() = msgids::MessageAll;

        for (int i = 0; i < command.size(); ++i)
            answer << static_cast<char>(command[i]);
        answer << command.size();

        std::cout << Client.name << ": " << command << '\n';
        Client.Send(answer);
    }
    else
    {
        if (command[1] == 'w' && command[2] == ' ')
        {
            std::string whisperName = command.substr(3, command.find(' ', 3) - 3);
            std::string whisperMessage = command.substr(command.find(' ', 3) + 1);
            Client.lastWhisper = whisperName;
            //std::cout << "name:" << whisperName << ", message:" << whisperMessage << '\n';

            answer.getID() = msgids::Whisper;
            
            for (int i = 0; i < whisperMessage.size(); ++i)
                answer << static_cast<char>(whisperMessage[i]);
            answer << static_cast<uint32_t>(whisperMessage.size());

            for (int i = 0; i < whisperName.size(); ++i)
                answer << static_cast<char>(whisperName[i]);
            answer << static_cast<uint32_t>(whisperName.size());

            Client.Send(answer);
            color(5);
            std::cout << "(Whisper To) " << whisperName << ": " << whisperMessage << '\n';
            color(7);
        }
        else if (command.substr(1, 8) == "confetti")
        {
            answer.getID() = msgids::Confetti;
            Client.Send(answer);
            std::cout << Client.name;
            color(2);
            std::cout << " threw ";
            color(5);
            std::cout << "confetti";
            color(9);
            std::cout << " all ";
            color(1);
            std::cout << "over";
            color(3);
            std::cout << " the ";
            color(4);
            std::cout << "chat";
            color(2);
            std::cout << " room!\n";
            color(7);
        }
        else if (command[1] == 'r' && command[2] == ' ')
        {
            if(Client.lastWhisper.empty())
            {
                color(8); std::cout << "There is noone to reply to...\n"; color(7);
            }
            else
            {
                std::string whisperMessage = command.substr(3);
                answer.getID() = msgids::Whisper;

                for (int i = 0; i < whisperMessage.size(); ++i)
                    answer << static_cast<char>(whisperMessage[i]);
                answer << static_cast<uint32_t>(whisperMessage.size());

                for (int i = 0; i < Client.lastWhisper.size(); ++i)
                    answer << static_cast<char>(Client.lastWhisper[i]);
                answer << static_cast<uint32_t>(Client.lastWhisper.size());

                Client.Send(answer);
                color(5);
                std::cout << "(Whisper To) " << Client.lastWhisper << ": " << whisperMessage << '\n';
                color(7);

            }
        }
        else { color(8); std::cout << "Incorrect Syntax.\n"; color(7); }

    }
}
