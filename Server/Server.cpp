// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <uwebsockets/App.h>
#include <map>
#include <string>

//#include <regex>
//#include <string>

using namespace std;

struct UserConnection
{
    string name;
    unsigned long userId;
};

void newUserConected(string channel, auto* ws, string name, unsigned long userId)
{
    string_view message("NEW_USER=" + to_string(userId) + "," + name);
    ws->publish(channel, message);
}

int main()
{
    int port = 8888;
    unsigned long lastUserId = 10;
    map<unsigned long, string> onlineUsers;

   uWS::App().ws<UserConnection>("/*", {
            .open = [&lastUserId, &onlineUsers](auto* ws)
            {
                UserConnection* data = ws->getUserData();
                data->userId = lastUserId++;
                data->name = "UNAMED";

                cout << "new client is connected ID = " << data->userId << endl;
                ws->subscribe("broadcast");
                string userChannel("user#" + to_string(data->userId));
                ws->subscribe(userChannel);

                for (auto& entry: onlineUsers)
                {
                    newUserConected(userChannel, ws, entry.second, entry.first);
                }

                onlineUsers[data->userId] = data->name;
                
            },
            .message = [&onlineUsers](auto* ws, string_view message, uWS::OpCode)
            {
                string SET_NAME("SET_NAME=");
                string MESSAGE_TO("MESSAGE_TO=");
                UserConnection* data = ws->getUserData();

                cout << "New message received - " << message << endl;
                if (message.find(SET_NAME) == 0)
                {
                    cout << "User set thier name" << endl;
                    data->name = message.substr(SET_NAME.length());
                    onlineUsers[data->userId] = data->name;
                    newUserConected("broadcast", ws, data->name, data->userId);
                }

                if (message.find(MESSAGE_TO) == 0)
                {
                    cout << "User sends private message" << endl;
                    auto rest = string(message.substr(MESSAGE_TO.length()));
                    int commaPosition = rest.find(",");
                    auto id = rest.substr(0, commaPosition);
                    auto text = data->name+ ", " + rest.substr(commaPosition + 1);
                        ws->publish("user#" + string(id), text);
                }
            },
            .close = [&onlineUsers](auto* ws, int code, string_view message)
            {
                UserConnection* data = ws->getUserData();
                ws->publish("broadcast", "DISCONNECT=" + to_string(data->userId));
                onlineUsers.erase(data->userId);
            },

        }).listen(port, [port](auto* token)
        {
            if (token)
            {
                cout << "server started succesfully on port: " << port << endl;
            }
            else
            {
                cout << "Server failed to start" << endl;
            }
        }).run();

    //uWS::App aplication;

    //auto openHandler = [&lastUserId](auto* ws)
    //                    {
    //                        UserConnection* data = ws->getUserData();
    //                        data->userId = lastUserId++;
    //                        data->name = "UNAMED";

    //                        cout << "new client is connected ID = " << data->userId << endl;
    //                        ws->subscribe("broadcast");
    //                        ws->subscribe("user#" + to_string(data->userId));
    //                    };

    //auto messageHandler = [](auto* ws, string_view message, uWS::OpCode)
    //{
    //    string SET_NAME("SET_NAME=");
    //    string MESSAGE_TO("MESSAGE_TO=");
    //    UserConnection* data = ws->getUserData();
    //    //regex SET_NAME("SET_NAME=.*");
    //    //regex MESSAGE_TO("MESSAGE_TO=\d+,.*");
    //    cout << "New message received - " << message << endl;
    //    // smatch match_result;
    //    // regex_match(message, match_result, SET_NAME);
    //    if (message.find(SET_NAME) == 0)
    //    {
    //        cout << "User set thier name" << endl;
    //        data->name = message.substr(SET_NAME.length());
    //    }
    //    if (message.find(MESSAGE_TO) == 0)
    //    {
    //        cout << "User sends private message" << endl;
    //        auto rest = message.substr(MESSAGE_TO.length());
    //        int commaPosition = rest.find(",");
    //        auto id = rest.substr(0, commaPosition);
    //        auto text = rest.substr(commaPosition + 1);
    //        ws->publish("user#" + string(id), text);
    //    }
    //};

    //aplication.ws<UserConnection>("/*", {.open = openHandler,.message = messageHandler });


    //auto portCallback = [port](auto* token)
    //                    {
    //                        if (token)
    //                        {
    //                            cout << "server started succesfully on port: " << port << endl;
    //                        }
    //                        else
    //                        {
    //                            cout << "Server failed to start" << endl;
    //                        }
    //                    };

    //aplication.listen(port, portCallback);

    //aplication.run();
}

