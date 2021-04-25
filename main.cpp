#include <iostream>
#include "sha256.h"             // Нагрузка
#include "memory.h"
#include "string.h"
#include <algorithm>




long long getfreememory()
{
    long long returnValue;
    const int BUFFER_SIZE = 1000;
    char buffer[BUFFER_SIZE];
    FILE *fInput;
    int loop;
    char ch;
    returnValue = -1;
    fInput = fopen("/proc/meminfo","r");
    if (fInput != nullptr)
    {
      while (!feof(fInput))
      {
        fgets(buffer,BUFFER_SIZE-1,fInput);
        if (feof(fInput))
        {
          break;
        }
        buffer[BUFFER_SIZE-1] = 0;
        // Look for serial number
        if (strncmp(buffer,"MemFree:",8)==0)
        {
          // Extract mem free from the line.
          for(loop=0;loop<BUFFER_SIZE;loop++)
          {
            ch = buffer[loop];
            if (ch == ':')
            {
               returnValue = 0;
               continue;
            }
            if (ch == 0)
            {
                break;
            }
            if (returnValue >=0)
            {
               if (ch >='A')
               {
                  break;
               }
               if ((ch >='0') && (ch <='9'))
               {
                  returnValue = returnValue * 10 + (ch-'0');
               }
            }
          }
          break;
        }
      }
      fclose(fInput);
    }
    return returnValue * 1000;
}



inline long getTimeStamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

/* Выполняет задачи клиента
 * Мониторит область памяти, откуда получает сигналы что необходимо выполнить что-либо
 */
class Server {
public:
    Server()  {
    }

    /* В бесконечного цикле проверяется доступный запрос
     */
    void workServer() {
        int tmpCounter = 0;
        while(true) {
//            if(mem->m_ready) {
//                Table *t;
//                if(mem->readData(t))
//                    continue;
//                switch (*t->cmd) {
//                case '0': {
//                    std::cout << t->cmd;
//                    std::string h = sha256(std::string(t->cmd));
//                    char *d = static_cast<char *>(malloc(h.size()));
//                    strcpy(d, h.c_str());
//                    t->dataLength = h.size();
//                    t->data = d;
//                    break;
//                }
//                default:
//                    std::cout << "not implementated" << std::endl;
//                    break;
//                }

//                std::cout << "ready read server" << std::endl;
//                mem->m_ready = false;
            }


            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if(tmpCounter >= 100)
//                break;
            tmpCounter++;
        }
//    }
};


class Client {

    long countReq = 0;
public:
    Client() {
    }

    void workClient() {
        int tmpCounter = 0;
        while(true) {
            if(sysinfo.createSession()) {
                std::string str;

                countReq++;

            }


            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if(tmpCounter >= 100)
                break;
            tmpCounter++;
        }
    }
};



// TODO область вызова должна состоять из указателей на функции
/* Инициализация области вызова
 */
int initBlockAPI() {

    std::cout << SIZE_PFUNC_API << std::endl;

    void (*pFunc)(int, char**);
    pFunc = call;
    sysinfo.sizePFuncAPI = sizeof pFunc;
    sysinfo.sizeAPITable = sysinfo.sizePFuncAPI * COUNT_FIELD;
    size_t sz = getfreememory();
    if(static_cast<unsigned int>(sz) > sysinfo.sizeAPITable) {
//        sysinfo.api = new (void (*)(int, char**))[COUNT_FIELD];
        for(size_t i = 0; i < COUNT_FIELD; i++)
            sysinfo.api[i] = call;
        return 0;
    }
    std::cout << "error init block API" << std::endl;
    return 1;
}


/*
 * Вызов API
 * |CMD|COUNT_ARG|POINT|
 * CMD -> sizeof(funcPAPI)
 * COUNT_CMD -> int (минимум 2)
 * POINT -> char**
 *      0 обязательно инициализируемое место для кода(int) возврата - сюда поставляются данные возврата от сервера
 *      1 обязательно id потока отправившего
 *      2 аргументы если нужны
 *      ...
 */
int initMemory() {
    sysinfo.sizeBlockMessage = sysinfo.sizePFuncAPI + sizeof (int) + sizeof (char**);
    sysinfo.sizeMemory = sizeof(int) + sysinfo.sizeBlockMessage;                // Минимально необходимое место для кода возврата выполнения функции на сервере и самого сообщения
    size_t freeMemory = getfreememory();
    if(static_cast<unsigned int>(freeMemory) > sysinfo.sizeMemory) {
        // Доступно минимальное колличество памяти
        freeMemory *= 0.9;
        return sysinfo.getMemory(sysinfo.sizeMemory);
//        return sysinfo.getMemory(freeMemory);
    }
    std::cout << "error init block memory" << std::endl;
    return 1;
}

void readCmd()
{
    sysinfo.readRequest();
//    if(mem->m_ready) {
//        Table *t;
//        if(mem->readData(t))
//            return;
//        switch (*t->cmd) {
//        case '0': {
//            std::cout << t->cmd;
//            std::string h = sha256(std::string(t->cmd));
//            char *d = static_cast<char *>(malloc(h.size()));
//            strcpy(d, h.c_str());
//            t->dataLength = h.size();
//            t->data = d;
//            break;
//        }
//        default:
//            std::cout << "not implementated" << std::endl;
//            break;
//        }

//        std::cout << "ready read server" << std::endl;
//        mem->m_ready = false;
//    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

static enum STATE{STATE_INIT_API, STATE_INIT_MEMORY,
                  STATE_LAUNGTH_CLIENT, STATE_WORK,
                  STATE_COMPLATING, STATE_EXIT} state;



int main()
{

    Client client;
    std::thread thclient;

    int ret;
    while(true) {
        ret = 0;
        switch (state) {
        case STATE_INIT_API:
            ret = initBlockAPI();
            if(ret) {
                return ret;
            }
            state = STATE_INIT_MEMORY;
            break;
        case STATE_INIT_MEMORY:
            ret = initMemory();
            if(ret) {
                return ret;
            }
            state = STATE_LAUNGTH_CLIENT;
            break;
        case STATE_LAUNGTH_CLIENT:
            thclient = std::thread(&Client::workClient, client);
            state = STATE_WORK;
            break;
        case STATE_WORK:
            readCmd();
            //std::cout << "stop";
            break;
        case STATE_COMPLATING:
            break;
        case STATE_EXIT:
            break;
        }
    }



    int memfree = getfreememory();


    std::cout << memfree << std::endl;
    memfree *= 0.9;
    std::cout << memfree << std::endl;
    std::cout << "part:" << memfree%1000 << std::endl;
    char *test = (char*)malloc(memfree);
    memset(test, 0, memfree);


    std::cout << memfree << " " << test << std::endl;
    memfree = getfreememory();
    std::cout << memfree << std::endl;
    free(test);

    std::cout << getTimeStamp() << std::endl;
    Server srv;

    auto thrsrv = std::thread(&Server::workServer, srv);
    thrsrv.join();

    std::cout << "Server thread is completed" << std::endl;


    return 0;
}
