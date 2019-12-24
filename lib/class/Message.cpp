
#include <string>
#include <iostream>

class Message
{
private:


public:
    User user;
    std::string content;
    void construct()
    {
        this->content = "";
    }

    void addData(std::string data)
    {
        if (this->user.login == "")
        {
            this->user.login = data.substr(0,data.length()-1);//removes \n from end
            return;
        }

        if (this->user.password == "")
        {
            this->user.password = data.substr(0,data.length()-1);//removes \n from end
            return;
        }

        this->content+=data;

    }

    bool isAuthorized(const User *users, unsigned numberOfUsers)
    {
        for (unsigned i = 0; i < numberOfUsers; i++)
        {
            if (users[i].login == this->user.login && users[i].password == this->user.password)
            {
                return true;
            }
        }
        return false;
    }

};
