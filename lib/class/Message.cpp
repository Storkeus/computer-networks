
#include <string>
#include <iostream>
#include <fstream>

class Message
{
private:
    std::string isEnd;

    bool isFile(std::string filename)
    {
        std::ifstream ifile(filename);
        return (bool)ifile;
    }

    std::string getMessageBody()
    {
        return this->senderAddress + "\n" + this->reciverLogin + '@' + this->reciverServer + "\n" + this->content;
    }

    std::string getRandomFileName(std::string path)
    {
        std::string chars =
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

        std::string fileName;
        do
        {
            for (int i = 0; i < 10; i++)
            {
                fileName += chars[rand() % chars.length()];
            }
        } while (isFile(path + fileName));

        return fileName;
    }

    void save(std::string path)
    {
        std::ofstream messageFile(path + getRandomFileName(path));
        messageFile << this->getMessageBody() << std::endl;
        messageFile.close();

        std::cout<<this->content<<std::flush;
    }

public:
    User user;
    std::string content;
    std::string reciverLogin;
    std::string reciverServer;
    std::string senderAddress;

    void construct()
    {
        this->content = "";
        this->reciverLogin = "";
        this->reciverServer = "";
        this->senderAddress = "";
        this->isEnd = "";
    }

    bool isAtReciverServer()
    {
        return this->isEnd == "1";
    }

    void addData(std::string data)
    {

        if (this->isEnd == "")
        {
            this->isEnd = data.substr(0, data.length() - 1); //removes \n from end
            return;
        }

        std::cout << "Obsługa połączenia. Typ: " << this->isEnd << std::endl
                  << std::flush;
        if (!this->isAtReciverServer())
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
        }
        if (this->senderAddress == "")
        {

            this->senderAddress = data.substr(0, data.length() - 1); //removes \n from end
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

        if (this->senderAddress == "")
        {
            this->senderAddress = data.substr(0, data.length() - 1); //removes \n from end
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

    void send()
    {
        std::cout << "Sending message to reciver..." << this->reciverServer.c_str()<< std::flush;
        int fd = socket(PF_INET, SOCK_STREAM, 0);

        struct hostent *host;
        host = gethostbyname(this->reciverServer.c_str());

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(atoi("1234"));
        memcpy(&addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

        connect(fd, (struct sockaddr *)&addr, sizeof(addr));

        std::string message = "1\n" + this->getMessageBody();
        std::cout << message << std::flush;
        write(fd, message.c_str(), message.length());

        close(fd);
    }
};
