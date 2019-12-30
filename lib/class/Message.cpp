
#include <string>
#include <iostream>
#include <fstream>

class Message
{
private:

bool isFile(std::string filename)
{
  std::ifstream ifile(filename);
  return (bool)ifile;
}

    std::string getRandomFileName(std::string path)
    {
       std::string chars =
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

        std::string fileName;
        do
        {
            for(int i=0; i<10; i++)
            {
                fileName+=chars[rand()%chars.length()];
            }
        } while (isFile(path+fileName));

        return fileName;
    }

    void save(std::string path)
    {
        std::ofstream messageFile(path+getRandomFileName(path));
        messageFile << this->reciverLogin << '@' << this->reciverServer << std::endl;
        messageFile << this->content << std::endl;
        messageFile.close();
    }

public:
    User user;
    std::string content;
    std::string reciverLogin;
    std::string reciverServer;
    void construct()
    {
        this->content = "";
        this->reciverLogin = "";
        this->reciverServer = "";
    }

    void addData(std::string data)
    {
        if (this->user.login == "")
        {
            this->user.login = data.substr(0, data.length() - 1); //removes \n from end
            return;
        }

        if (this->user.password == "")
        {
            this->user.password = data.substr(0, data.length() - 1); //removes \n from end
            return;
        }

        if (this->reciverServer == "")
        {
            std::string reciverAddress = data.substr(0, data.length() - 1); //removes \n from end
            std::string login;
            std::string server;

            bool isFirstPart = true;
            for (unsigned i = 0; i < reciverAddress.length(); i++)
            {
                if (reciverAddress[i] == '@')
                {
                    isFirstPart = false;
                    continue;
                }

                if (isFirstPart)
                {
                    login += reciverAddress[i];
                }
                else
                {
                    server += reciverAddress[i];
                }
            }

            this->reciverLogin = login;
            this->reciverServer = server;

            return;
        }

        this->content += data;
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

    void saveOut()
    {
        std::string path = "./messages/" + this->user.login + "/out/";

        this->save(path);
    }

    void saveIn()
    {
        std::string path = "./messages/" + this->reciverLogin + "/in/";
        this->save(path);
    }
};
