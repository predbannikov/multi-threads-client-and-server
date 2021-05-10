#include "test.h"
#include <iterator>
#include <string.h>
#include <iomanip>
#include <algorithm>
#include "sha256.h"


TAbstract::~TAbstract() {
//    std::cout << "~TAbstract " << m_sName << ": " << std::hex << std::this_thread::get_id() << std::endl;

//    if(m_sName == "server") {
//        std::cout << std::endl << "Data" << std::hex << std::endl;
//        for(auto item: *messages->data) {
//                for(auto i: item)
//                    std::cout << (0xFF&i) << " ";
//                std::cout << std::endl;
//        }
//    }

}

int TServer::openSession(size_t anNumMessage)
{
    try {

//    std::cout << std::endl;
//    std::cout << "Server " << std::endl;
//    for(auto i: data->at(anNumMessage))
//        std::cout << (0xFF&i) << " ";
//    std::cout << std::endl;
//        std::cout << "openSession: " << std::hex << std::this_thread::get_id() << std::endl;

        char data[SIZE_BLOCK_MESSAGE];
        char *pData = &data[0];
//        std::shared_ptr<char> sptr(data);
        if(!messages->readMessage(pData)) {
//            std::cout << "not read message" << std::endl;
            return 0;
        }
//    std::lock_guard<std::mutex> lk(messages->mtx);
    int64_t intPart = *reinterpret_cast<int64_t*>(pData);
//    int64_t intPart = *reinterpret_cast<int64_t*>(&messages->data->at(anNumMessage).at(0));
    int16_t code = static_cast<int16_t>(intPart);
    int16_t reqNum = static_cast<int16_t>(intPart >>= 16);
    int16_t funcNum = static_cast<int16_t>(intPart >>= 16);

//    std::cout << "code: " << code << std::endl;

    int16_t countArgs = static_cast<int16_t>(intPart);

    //std::cout << "SIZE_PARAMETERS_INT " << SIZE_PARAMETERS_INT << std::endl;
//    char **ptr = reinterpret_cast<char**>(*reinterpret_cast<char**>(&data->at(anNumMessage).at(SIZE_PARAMETERS_INT)));
//    long mark = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//    for(long i = 0; i < 500000000; i++) {
//        char **pTmp = reinterpret_cast<char**>(*reinterpret_cast<char**>(&data->at(anNumMessage).at(SIZE_PARAMETERS_INT)));
//    }
//    long dmark = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - mark;
//    std::cout << "1 dmark = " << std::dec << dmark << std::endl;

//    char& p = messages->data->at(anNumMessage).at(SIZE_PARAMETERS_INT);

//    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    void (TServer::*pFunc)(int16_t&, char**);
    pFunc = api[funcNum];   //TODO узнать почему в массиве функций изменяются значения
    char** ptr = nullptr;
    memcpy(&ptr, pData + SIZE_PARAMETERS_INT, sizeof(char*));
//    std::cout << "." << std::endl;
    (this->*pFunc)(countArgs, ptr);

    memcpy(pData + SIZE_PARAMETERS_INT, &ptr, sizeof(char**));

    messages->changeData(pData, reqNum, CODE_MESSAGE_READY);

    } catch(const std::runtime_error& re) {
        std::cerr << "Runtime error: " << re.what() << std::endl;
    } catch(const std::exception& ex) {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    } catch(...) {
        std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
    }
    

    return 0;
}

void TServer::call(int16_t &nArgs, char **ppchArgs){
    try {
        std::string *strtmp = reinterpret_cast<std::string*>(*ppchArgs);
        if(*strtmp == "stop server") {
            cycle = false;
        }
        std::string *strResponse = new std::string(sha256(strtmp));
    //    strings.push_back(strResponse);
        delete *ppchArgs;
        nArgs++;
        ppchArgs = new char*[static_cast<size_t>(nArgs)] {
                reinterpret_cast<char*>(strtmp),
                reinterpret_cast<char*>(strResponse)
        };
    } catch (...) {
        std::cerr << "Call: Unknown failure occurred. Possible memory corruption" << std::endl;
    }


}

int TAbstract::createRequest(InfoThread *infoThr, int16_t aCountArgs, char **appchData)
{
    //std::cout << "bt" << SIZE_CODE_MESSAGE << " " << SIZE_NUM_FUNC_API << std::endl;
    //std::lock_guard<std::mutex> lock(mtx);
//    std::cout << "createRequest: " << std::hex << std::this_thread::get_id() << std::endl;
    try {
        const int16_t numApiFunc = 1;
        const int16_t codeMessage = CODE_MESSAGE_NEW ;                                            // CODE
        const int16_t countArgs = aCountArgs;
        const int16_t numRequest = infoThr->numRequest;
        int64_t prepFuncArgs = numApiFunc;
        prepFuncArgs <<= 16;             // Пустое место - номер запроса
        prepFuncArgs |= countArgs;
        prepFuncArgs <<= 16;
        prepFuncArgs |= numRequest;
        prepFuncArgs <<= SIZE_NUM_FUNC_API * 8;
        prepFuncArgs |= codeMessage;
        char data[SIZE_BLOCK_MESSAGE];
        //char *pData = &data[0];     //new char[SIZE_BLOCK_MESSAGE];
        memcpy(data, &prepFuncArgs, SIZE_PARAMETERS_INT);  // функция + RET + ARGS
    //    const size_t indexTopData = getIndexBlockReady();
        //std::cout << "index block ready = " << std::hex << indexTopData << std::endl;
//        std::lock_guard<std::mutex> lk(messages->mtx);
//        memcpy(messages->data->at(infoThr->indxMessage).data(), &prepFuncArgs, SIZE_PARAMETERS_INT);  // функция + RET + ARGS
    //    char **ppchTmp = new char*[static_cast<size_t>(1)] {
    //            reinterpret_cast<char*>(new size_t(anNumReq))
    //    };
        memcpy(data + SIZE_PARAMETERS_INT, &appchData, sizeof appchData);
        messages->addMessage(data);


        try {
            std::unique_lock<std::mutex> lk(infoThr->mtx);

        } catch (...) {
            std::cerr << "unique_lock before contition_variable proc" << std::endl;
        }
        std::unique_lock<std::mutex> lk(infoThr->mtx);
    //    std::vector<char> &ptr = messages->data->at(infoThr->indxMessage);
        infoThr->condVar.wait(lk, [infoThr, this] (){
//            int16_t index;
//            messages->getIndexOfNumReq(infoThr->numRequest, index);
//            if(index < 0)
//                return false;
            bool bl = this->messages->checkRequest(infoThr->numRequest, CODE_MESSAGE_READY);            // CODE
//            std::cout << "wait code = " << bl << " req = " << infoThr->numRequest << std::endl;
            return  bl;

        });

    //    infoThr->complate = 1;
//        int16_t index;
//        messages->getIndexOfNumReq(numRequest, index);
        messages->changeData(data, infoThr->numRequest, CODE_MESSAGE_COMPL);                            // CODE
//        std::cout << "end creat: numReq = " << std::dec << numRequest << std::endl;


    }catch(const std::runtime_error& re) {
        std::cerr << "Runtime error: " << re.what() << std::endl;
    } catch(const std::exception& ex) {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    } catch(...) {
        std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
    }


//    std::this_thread::sleep_for(std::chrono::milliseconds(1050));
//    std::cout << "end creat request befor" << std::endl;


    return 0;
}

int TAbstract::createRequestStopServer(InfoThread *infoThr, int16_t aCountArgs, char **appchData)
{
    try {

        const int16_t numApiFunc = 1;
        const int16_t codeMessage = CODE_MESSAGE_NEW ;                  //MESSAGE_FILLED;
        const int16_t countArgs = aCountArgs;
        const int16_t numRequest = infoThr->numRequest;
        int64_t prepFuncArgs = numApiFunc;
        prepFuncArgs <<= 16;             // Пустое место - номер запроса
        prepFuncArgs |= countArgs;
        prepFuncArgs <<= 16;
        prepFuncArgs |= numRequest;
        prepFuncArgs <<= SIZE_NUM_FUNC_API * 8;
        prepFuncArgs |= codeMessage;
        char data[SIZE_BLOCK_MESSAGE];
        memcpy(data, &prepFuncArgs, SIZE_PARAMETERS_INT);  // функция + RET + ARGS
        memcpy(data + SIZE_PARAMETERS_INT, &appchData, sizeof appchData);
        messages->addMessage(data);

        std::unique_lock<std::mutex> lk(infoThr->mtx);
        infoThr->condVar.wait(lk, [infoThr, this] (){
            bool bl = this->messages->checkRequest(infoThr->numRequest, CODE_MESSAGE_READY);
            return  bl;

        });

        messages->changeData(data, infoThr->numRequest, CODE_MESSAGE_COMPL);
        std::cout << "end creat stop server: numReq = " << std::dec << numRequest << std::endl;
    } catch (...) {
        std::cerr << "Create Request Stop Server: Unknown failure occurred. Possible memory corruption" << std::endl;
    }
    return 0;
}


//union SizetToBytesUnion {
//  size_t value;
//  unsigned char bytes[sizeof(value)];
//} u;
//size_t value = 0xFEFEFEF;
////SizetToBytesUnion u;
//u.value = value;
//size_t zeroBitsN = 0;
//for (size_t i = 0; i != sizeof(u.bytes); ++i)
//  zeroBitsN |= u.bytes[i];

/************************************************
 *                  Server
 * **********************************************/
TServer::TServer(DataMessages *messages_) : TAbstract(messages_) {
    m_sName = "server";
    for(size_t i = 0; i < COUNT_FIELD; i++)
        api[i] = &TServer::call;

    t = std::thread(&TServer::worker, this);
    if(!t.joinable())
        throw std::logic_error("No thread");



}

int TServer::writeMessage() {
    std::cout << "Server write" << std::endl;
    return 0;
}

int TServer::readMessage() {
    return 0;
}

TServer::~TServer() {
    try{

        t.join();

        std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
        threads.clear();

        for(size_t i = 0; i < strings.size(); i++) {
            delete strings[i];
        }
        strings.clear();

        delete messages;
        std::cout << "~" << m_sName << "()" << std::endl;


    } catch(const std::runtime_error& re) {
        std::cerr << "Runtime error: " << re.what() << std::endl;
    } catch(const std::exception& ex) {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    } catch(...) {
        std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
    }
}

int TServer::worker()
{
    try {

        std::cout << "start " << m_sName << ": " << std::hex << std::this_thread::get_id() << std::endl;
    //    long tmp_counter = 0;
        //std::shared_ptr<scoped_guard> scope_guard_share;
        while(cycle) {
    //        const long indexData = tmp_counter; //messages->getIndexBlockFilled();
    //        if(indexData < 0) {
    //            std::this_thread::sleep_for(std::chrono::milliseconds(150));
    //            continue;
    //        }

            // Дальше проходим только если в очереди есть есть необработанные блоки
            if(messages->messagesIsFilled()) {
                try {
                    threads.push_back(std::thread(&TServer::openSession, this, 1));

                } catch (...) {
                    std::cerr << "TServer::worker: threads.push_back() size = " << std::dec << threads.size() << std::endl;
                    return 0;
                }

            }


    //        std::this_thread::sleep_for(std::chrono::milliseconds(3));

        }
    //    std::cout << "\nServer worker end" << std::endl;
    } catch (...) {
        std::cerr << "TServer::worker: Unknown failure occurred. Possible memory corruption" << std::endl;
    }
    return 0;
}


/************************************************
 *                  Client
 * **********************************************/
TClient::TClient(DataMessages *messages_) :  TAbstract(messages_) {
    m_sName = "client";
    t = std::thread(&TClient::worker, this);
    if(!t.joinable())
        throw std::logic_error("No thread");


}

#define COUNT_MAX_REQUEST   100

int TClient::worker()
{


    try {

        std::cout << "start " << m_sName << ": " << std::hex << std::this_thread::get_id() << std::endl;

        int16_t tmp_counter = 1;
        std::string *strTmp;
        char **ppchTmp = nullptr;
        InfoThread *infoThr;
        while(cycle) {
            int16_t code;
            int16_t req = -1;
            if(!messages->getMessageReady(code, req) && !complate) {
                if(!messages->getMessageInfo(code, req)) {
                    strTmp = new std::string("hello world");
                    ppchTmp = new char*[static_cast<size_t>(1)] {    // количество указателей (аргументов)
                            reinterpret_cast<char*>(strTmp)
                    };
                    infoThr = new InfoThread;
                    infoThr->numRequest = tmp_counter;
                    infoThr->threadGuard = new scoped_guard(std::thread(&TClient::createRequest, this, infoThr, 1, ppchTmp));       // передаются ссылки
                    queueThreads.insert(std::make_pair(infoThr->numRequest, infoThr));
                    tmp_counter++;
                    if(tmp_counter >= COUNT_MAX_REQUEST)
                        complate = true;
                    continue;
                }
                switch (code) {
                case CODE_MESSAGE_EMPTY:                        // Если пустые
                    strTmp = new std::string("hello world");
                    ppchTmp = new char*[static_cast<size_t>(1)] {    // количество указателей (аргументов)
                            reinterpret_cast<char*>(strTmp)
                    };
                    infoThr = new InfoThread;
                    infoThr->numRequest = tmp_counter;
                    infoThr->threadGuard = new scoped_guard(std::thread(&TClient::createRequest, this, infoThr, 1, ppchTmp));       // передаются ссылки
                    queueThreads.insert(std::make_pair(infoThr->numRequest, infoThr));
                    tmp_counter++;
                    if(tmp_counter >= COUNT_MAX_REQUEST)
                        complate = true;

                    //                        std::cout << "map insert " << infoThr->numRequest << std::endl;

                    break;


                }
                continue;
            }
            // ------------------------------------------------------------

            switch (code) {
            case CODE_MESSAGE_READY:
    //            std::cout << "notify_one: req = " << req << std::endl;
                queueThreads[req]->condVar.notify_one();

                break;
            case CODE_MESSAGE_COMPL:

                messages->clearMessage(req);
                queueThreads.erase(req);
                if(queueThreads.empty() && complate) {
    //                cycle = false;
                }

                break;
            }


            if(queueThreads.empty() && tmp_counter == COUNT_MAX_REQUEST) {
                strTmp = new std::string("stop server");
                ppchTmp = new char*[static_cast<size_t>(1)] {    // количество указателей (аргументов)
                        reinterpret_cast<char*>(strTmp)
                };
                infoThr = new InfoThread;
                infoThr->numRequest = tmp_counter;
                infoThr->threadGuard = new scoped_guard(std::thread(&TClient::createRequestStopServer, this, infoThr, 1, ppchTmp));       // передаются ссылки
                queueThreads.insert(std::make_pair(infoThr->numRequest, infoThr));
                tmp_counter++;
                if(tmp_counter >= COUNT_MAX_REQUEST)
                    complate = true;

    //            break;
            }
    //        std::this_thread::sleep_for(std::chrono::milliseconds(8));
            if(code == 0 && req == 0 && queueThreads.empty())
                break;

        }

    } catch(const std::runtime_error& re) {
        std::cerr << "Runtime error: " << re.what() << std::endl;
    } catch(const std::exception& ex) {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    } catch(...) {
        std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
    }



//    std::cout << "\nClient worker end" << std::endl;
    return 0;
}

int TClient::writeMessage() {


    std::cout << "Client write" << std::endl;
    return 0;
}

int TClient::readMessage() {
    return 0;
}

TClient::~TClient() {
    t.join();
//    std::for_each(queueThreads.begin(), queueThreads.end(), [](std::pair<const unsigned long, InfoThread*> &pair) {
//        std::cout << &pair.second->thread << std::endl;
//         pair.second->thread.join(); //
////         std::mem_fn(&std::thread::join);
//    });
//    std::cout << "delete ~" << m_sName << "() queuetThreads" << std::endl;
    try {


        for(auto info: queueThreads) {
            delete info.second;
        }
        queueThreads.clear();
        std::cout << "~" << m_sName << "()" << std::endl;


    } catch(const std::runtime_error& re) {
        std::cerr << "Runtime error: " << re.what() << std::endl;
    } catch(const std::exception& ex) {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    } catch(...) {
        std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
    }


}
static int i = 0;

void test2() {
    i++;
    if(i > 1000000000)
        return;
    test2();
}

class A {
    static int in;
public:
    A(int i = 0){in++; std::cout << "construct A" << std::endl;}
    ~A(){std::cout << "destruct A" << std::endl;}
    A(const A&a){std::cout << "copy construct A" << std::endl;}
    A& operator=(const A&a){std::cout << "assignment A" << std::endl; return *this;}
    void print() const {std::cout << this << " print "  << std::endl;}
};
int A::in = 0;//    long mark = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

void test3(std::vector<std::vector<char >> &vec2) {
    long mark = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    while(true){

        try {

        }catch(const std::runtime_error& re)
        {
            std::cerr << "Runtime error: " << re.what() << std::endl;
        }
        catch(const std::exception& ex)
        {
            std::cerr << "Error occurred: " << ex.what() << std::endl;
        }
        catch(...)
        {
            std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
        }
        long mark2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        long dmark = mark2 - mark;
        if(dmark > 5)
            break;
    }
    std::cout << "stop test" << std::endl;
}

int test() {


    //std::vector<std::vector<char >> *vec = new std::vector<std::vector<char >>(0, std::vector<char>(SIZE_BLOCK_MESSAGE));
    DataMessages *messages = new DataMessages;

//    std::thread thr = std::thread(&TServer::worker, server);
    TServer server(messages);
    TClient client(messages);


//    std::cout << "do something in current thread" << std::endl;

//    delete messages;
    return 0;
}

size_t DataMessages::getIndexBlockReady()
{
    for(size_t i = 0; i < data->size(); i++) {
        if(*reinterpret_cast<int16_t*>(&data->at(i).at(0)) == CODE_MESSAGE_EMPTY) {     // Код свободного блока сообщения
            return i;
        }
    }
    data->push_back(std::vector<char>(SIZE_BLOCK_MESSAGE, 0));
    return data->size()-1;
}

long DataMessages::getIndexBlockFilled()
{
    try {
        for(size_t i = 0; i < data->size(); i++) {
            int16_t code = *reinterpret_cast<int16_t*>(&data->at(i)[0]);
            if(code == CODE_MESSAGE_NEW) {
                data->at(i).at(0) = static_cast<char>(0x55);
                return static_cast<long>(i);
            }

        }
    } catch (...) {
        std::cout << "exception getIndexBlockFilled" << std::endl;
    }
    return -1;
}

bool DataMessages::messagesIsFilled()
{
    try {
        std::lock_guard<std::mutex> lk(mtx);
        for(size_t i = 0; i < data->size(); i++) {
            int16_t code = *reinterpret_cast<int16_t*>(&data->at(i)[0]);
            if(code == CODE_MESSAGE_NEW) {
                return true;
            }

        }
    } catch (...) {
        std::cout << "exception getIndexBlockFilled" << std::endl;
    }
    return false;
}

/*
 * Проверка кода по номеру запроса
 */
bool DataMessages::checkRequest(int16_t numReq, int16_t check_code)
{
    try {
        std::lock_guard<std::mutex> lk(mtx);
        for(size_t i = 0; i < data->size(); i++) {
            int16_t check_request = *reinterpret_cast<int16_t*>(&data->at(i)[2]);
            if(check_request == numReq) {
                int16_t code = *reinterpret_cast<int16_t*>(&data->at(i)[0]);
                return check_code == code;
            }
        }
    } catch (...) {
        std::cout << "exception checkRequest" << std::endl;;
    }
    return false;
}


bool DataMessages::getMessageInfo(int16_t &code, int16_t &req)
{
    try {
        code = -1;
        req = -1;
        std::lock_guard<std::mutex> lk(mtx);

        for(size_t i = 0; i < data->size(); i++) {
            code = *reinterpret_cast<int16_t*>(&data->at(i)[0]);
            req = *reinterpret_cast<int16_t*>(&data->at(i)[2]);                                     // Продолжить
            switch (code) {
            case CODE_MESSAGE_EMPTY:
                return true;
            }
//            if(code == CODE_MESSAGE_EMPTY) {
//                return true;
//            }

        }
    } catch (...) {
        std::cout << "exception getMessageInfo" << std::endl;
    }
    return false;
}

bool DataMessages::getMessageReady(int16_t &code, int16_t &req)
{
    code = -1;
    req = -1;
    std::lock_guard<std::mutex> lk(mtx);
    for(size_t i = 0; i < data->size(); i++) {
        code = *reinterpret_cast<int16_t*>(&data->at(i)[0]);
        req = *reinterpret_cast<int16_t*>(&data->at(i)[2]);                                     // Продолжить
        switch (code) {
        case CODE_MESSAGE_COMPL:
        case CODE_MESSAGE_READY:
            return true;
        }
    }
    return false;
}

bool DataMessages::isMessageEmpty()
{

}

void DataMessages::getIndexOfReqNum(int16_t req, int16_t &index)
{
    try {
        std::lock_guard<std::mutex> lk(mtx);
        for(size_t i = 0; i < data->size(); i++) {
            int16_t check_request = *reinterpret_cast<int16_t*>(&data->at(i)[2]);
            if(check_request == req) {
                index = static_cast<int16_t>(i);
                return;
            }
        }
    } catch (...) {
        std::cout << "exception getIndexOfNumReq";
    }
    index = -1;
}

bool DataMessages::setCodeOfReqNum(int16_t req, int16_t code)
{
    try {
        std::lock_guard<std::mutex> lk(mtx);
        for(size_t i = 0; i < data->size(); i++) {
            int16_t check_request = *reinterpret_cast<int16_t*>(&data->at(i)[2]);
            if(check_request == req) {
                *reinterpret_cast<int16_t*>(&data->at(i)[0]) = code;
                return true;
            }
        }
    } catch (...) {
        std::cout << "exception setCodeOfReqNum";
    }
    return false;
}

bool DataMessages::clearMessage(int16_t req)
{
    try {

        std::lock_guard<std::mutex> lk(mtx);
        for(size_t i = 0; i < data->size(); i++) {
            int16_t check_request = *reinterpret_cast<int16_t*>(&data->at(i)[2]);
            if(check_request == req) {
                memset(data->at(static_cast<size_t>(i)).data(), 0, SIZE_BLOCK_MESSAGE);
                return true;
            }
        }


    }catch(const std::runtime_error& re)
    {
        std::cerr << "Runtime error: " << re.what() << std::endl;
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
    }
    return false;
}

/*
 * Поиск индекса сообщения в vector'e по номеру запроса 'req'
 * и запись данных из 'ptrData' по номеру этого индекса.
 * В 'ptrData' предварительно заносится 2 байтный код сообщения.
 * Потокозащищенное.
 */
bool DataMessages::changeData(char *ptrData, int16_t req, int16_t code_)
{
    try {

        std::lock_guard<std::mutex> lk(mtx);
        for(size_t i = 0; i < data->size(); i++) {
            int16_t check_request = *reinterpret_cast<int16_t*>(&data->at(i)[2]);
            if(check_request == req) {
                *reinterpret_cast<int16_t*>(ptrData) = static_cast<int16_t>(code_);
                memcpy(data->at(static_cast<size_t>(i)).data(), ptrData, SIZE_BLOCK_MESSAGE);
                return true;
            }
        }


    }catch(const std::runtime_error& re)
    {
        std::cerr << "Runtime error: " << re.what() << std::endl;
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
    }

}

void DataMessages::addMessage(char *ptrData)
{
    try {
        // Получить индекс для записи
        // Скопировать ptrData в data
        std::lock_guard<std::mutex> lk(mtx);
        size_t index = getIndexBlockReady();
    //    std::cout << "SIZE_BLOCK_MESSAGE " << SIZE_BLOCK_MESSAGE << std::endl;
        memcpy(data->at(index).data(), ptrData, SIZE_BLOCK_MESSAGE);

    }catch(const std::runtime_error& re)
    {
        std::cerr << "Runtime error: " << re.what() << std::endl;
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
    }


}

bool DataMessages::readMessage(char *&ptrData)
{
    try{
        // Получить индекс для чтения
        // скопировать данные в ptrData из data
        std::lock_guard<std::mutex> lk(mtx);
        long index_ = getIndexBlockFilled();
        if(index_ >= 0) {
            memcpy(ptrData, data->at(static_cast<size_t>(index_)).data(), SIZE_BLOCK_MESSAGE);
            return true;
        }

    }catch(const std::runtime_error& re)
    {
        std::cerr << "Runtime error: " << re.what() << std::endl;
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    } catch(...)
    {
        std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
    }


    return false;
}
