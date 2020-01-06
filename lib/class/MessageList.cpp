
#include <string>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <algorithm>
enum MessageListType
{
    UNDEFINED,
    INBOX = 2,
    OUTBOX = 3
};

bool compareMessage(const ResponseMessage &a, const ResponseMessage &b)
{
    return a.created_at > b.created_at;
}

class MessageList
{
private:
    MessageListType type;
    std::vector<ResponseMessage> list;

    bool addMessageToList(std::string filePath, std::string name)
    {
        struct stat fileStats;
        stat(filePath.c_str(), &fileStats);
        ResponseMessage message;
        message.name = name;
        message.created_at = fileStats.st_mtime;
        this->list.push_back(message);

        return true;
    }

    void sortList()
    {
        std::sort(this->list.begin(), this->list.end(), compareMessage);
    }

    std::string getListAsText(std::string directory)
    {
        std::string response = "";
        for (std::size_t i = 0; i < this->list.size(); ++i)
        {
            std::ifstream fileMessage;
            fileMessage.open(directory + "/" + this->list[i].name);
            std::string message = "";
            //    std::string  created_at= ctime((const time_t*)this->list[i].created_at);
            struct tm *timeinfo =
                localtime(&this->list[i].created_at);
                     std::ostringstream date;

            date<<timeinfo->tm_mday<<"."<<timeinfo->tm_mon+1 <<"."<<timeinfo->tm_year+1900<<" "<<timeinfo->tm_hour<<":"<<timeinfo->tm_min<<":"<<timeinfo->tm_sec<<'\n';
            response+=date.str();
            if (fileMessage.is_open())
            {
                while (!fileMessage.eof())
                {
                    fileMessage >> message;
                    response += message+'\n';
                }
            }
            response += '\n';
        }

        return response;
    }

public:
    User user;

    MessageList()
    {
        this->type = UNDEFINED;
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

    void addData(std::string data)
    {

        std::cout << this->type << std::endl
                  << std::flush;
        if (this->type == UNDEFINED)
        {
            std::cout << "Typ:" << data << std::flush;
            std::cout << this->type << std::endl
                      << std::flush;
            this->type = (MessageListType)(data[0] - '0'); //removes \n from end
            std::cout << this->type << std::endl
                      << std::flush;
            return;
        }

        if (this->user.login == "")
        {
            std::cout << "Login:" << data << std::flush;
            this->user.login = data.substr(0, data.length() - 1); //removes \n from end
            return;
        }

        if (this->user.password == "")
        {
            std::cout << "Hasło:" << data << std::flush;
            this->user.password = data.substr(0, data.length() - 1); //removes \n from end
            return;
        }
    }

    std::string get()
    {

        std::string directory = "./messages/" + this->user.login;
    directory+=this->type==INBOX?"/in":"/out";
        DIR *dirp = opendir(directory.c_str());
        struct dirent *dp;

        while ((dp = readdir(dirp)) != NULL)
        {
            std::string messageFile = dp->d_name;

            if (messageFile != ".." && messageFile != ".")
            {

                this->addMessageToList(directory + "/" + messageFile, messageFile);
            }
        }
        closedir(dirp);

        this->sortList();

        return this->getListAsText(directory);
    }
};
