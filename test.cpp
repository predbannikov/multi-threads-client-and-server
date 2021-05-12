#include "test.h"
#include <iterator>
#include <string.h>
#include <iomanip>
#include <algorithm>
#include "sha256.h"


#define COUNT_MAX_REQUEST       500
#define COUNT_MAX_THREADS       16




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

int TServer::openSession(ThreadInfo2 *infoThr)
{



    try {
        char data[SIZE_BLOCK_MESSAGE];
        char *pData = &data[0];
        while(infoThr->cycle) {


            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            if(!messages->readMessage(pData)) {
                //            std::cout << "not read message" << std::endl;
                std::this_thread::yield();
                continue;
            }
            int64_t intPart = *reinterpret_cast<int64_t*>(pData);
            //    int64_t intPart = *reinterpret_cast<int64_t*>(&messages->data->at(anNumMessage).at(0));
            int16_t code = static_cast<int16_t>(intPart);
            int16_t reqNum = static_cast<int16_t>(intPart >>= 16);
            int16_t countArgs = static_cast<int16_t>(intPart >>= 16);
            int16_t funcNum = static_cast<int16_t>(intPart >>= 16);



            void (TServer::*pFunc)(int16_t&, char**);
            pFunc = api[funcNum];

            char** ptr = nullptr;
            memcpy(&ptr, pData + SIZE_PARAMETERS_INT, sizeof(char*));
            (this->*pFunc)(countArgs, ptr);

//            memcpy(pData + SIZE_PARAMETERS_INT, &ptr, sizeof(char**));
            memcpy(pData + 4, &countArgs, sizeof (countArgs));

            messages->changeData(pData, reqNum, CODE_MESSAGE_READY);

        }

    } catch(const std::runtime_error& re) {
        std::cerr << "Runtime error: " << re.what() << std::endl;
    } catch(const std::exception& ex) {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    } catch(...) {
        std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
    }
    

    return 0;
}

void TServer::call0(int16_t &nArgs, char **ppchArgs){
    try {
        std::string allString;
        std::string *strtmp = nullptr;
        for(int16_t i = 0; i < nArgs; i++) {
            strtmp = reinterpret_cast<std::string*>(*(ppchArgs+i));
            allString.append(*strtmp);
        }
//        std::cout << "---------------------------" << std::endl;
        std::string *strResponse = new std::string;
        *strResponse = sha256(&allString);
        nArgs++;
        *(ppchArgs + nArgs - 1) = reinterpret_cast<char *>(strResponse);

//        std::cout << *strResponse << " - strResponse nArgs =" << nArgs << std::endl;
//        call2(nArgs, ppchArgs);

//        strings.push_back(strResponse);
//        delete ppchArgs;
//        ppchArgs = nullptr;


    } catch(const std::runtime_error& re) {
        std::cerr << "call0: Runtime error: " << re.what() << std::endl;
    } catch(const std::exception& ex) {
        std::cerr << "call0: Error occurred: " << ex.what() << std::endl;
    } catch(...) {
        std::cerr << "call0: Unknown failure occurred. Possible memory corruption" << std::endl;
    }


}

void TServer::call1(int16_t &nArgs, char **ppchArgs)
{
//    std::lock_guard<std::mutex>lg(mtx);
    cycle = false;
    next = true;
    for(size_t i = 0; i < threads.size(); i++)
        threads[i]->cycle = false;
    condVar.notify_one();
    std::cout << "call1 stop server" << std::endl;
}

void TServer::call2(int16_t &nArgs, char **ppchArgs)
{
    std::cout << *reinterpret_cast<std::string*>(*(ppchArgs+nArgs)) << " :nArgs=" << std::dec << nArgs <<  std::endl;

//    for(int16_t i = 0; i < nArgs; i++) {
//        std::cout << *reinterpret_cast<std::string*>(*(ppchArgs+i)) << std::endl;
//    }
}

void TServer::call3(int16_t &nArgs, char **ppchArgs)
{
    thread_count = *reinterpret_cast<size_t*>(*ppchArgs);
    next = true;
    condVar.notify_one();
}

#define COUNT_ITERATION_HASH    128       // не меньше 5 иначе будет всё равно минимум пять указателей на данные

int TClient::createRequest(InfoThread *infoThr, int16_t aCountArgs, char **appchData)
{
    try {
        long mark1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//        long deltaTime = 0;
        STATE_REQUEST stateReq = STATE_REQ_INIT;
        const int16_t numApiFunc = 0;
        const int16_t codeMessage = CODE_MESSAGE_NEW ;
        const int16_t countArgs = 1;
        const int16_t numRequest = infoThr->numRequest;

//        int64_t intPart;
//        int16_t code;
//        int16_t reqNum;
//        int16_t countArgsRet;
//        int16_t funcNum;

        int64_t paramInt64 = numApiFunc;
        std::string *strRequest;
        char **ppchTmp = nullptr;
        char** ptr = nullptr;
        char data[SIZE_BLOCK_MESSAGE];
        char *pData;
        InfoOfWork *iow;
        const size_t countPointerArgs = COUNT_ITERATION_HASH + 3;       //
        size_t counter = 0;
        while(infoThr->cycle) {
            switch (stateReq) {
            case STATE_REQ_INIT:
                paramInt64 <<= 16;
                paramInt64 |= countArgs;
                paramInt64 <<= 16;
                paramInt64 |= numRequest;
                paramInt64 <<= 16;
                paramInt64 |= codeMessage;
                memcpy(data, &paramInt64, SIZE_PARAMETERS_INT);
                strRequest = new std::string;
                *strRequest = std::to_string(infoThr->numRequest);
                ppchTmp = new char*[static_cast<size_t>(countPointerArgs)] {
                        reinterpret_cast<char*>(strRequest)
                };
                memcpy(data + SIZE_PARAMETERS_INT, &ppchTmp, sizeof ppchTmp);
                messages->addMessage(data);
                stateReq = STATE_REQ_WORK;
                break;
            case STATE_REQ_WORK:
                messages->changeCode(infoThr->numRequest, CODE_MESSAGE_NEW);
                counter++;
                if(counter == COUNT_ITERATION_HASH)
                    stateReq = STATE_REQ_PRINT;

                break;
            case STATE_REQ_PRINT:
                iow = new InfoOfWork;
                iow->timePerform = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - mark1;
                iow->timeSinceLastTask = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - lastTaskComplate;
                lastTaskComplate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                strRequest = new std::string;
                strRequest->append("Time=");
                strRequest->append(std::to_string(iow->timePerform));
                strRequest->append("\tdTask=").append(std::to_string(iow->timeSinceLastTask));
                strRequest->append("\treq=");
                strRequest->append(std::to_string(infoThr->numRequest));
                messages->changeApiFunc(infoThr->numRequest, 2);
                *(ppchTmp + countPointerArgs - 1) = reinterpret_cast<char*>(strRequest);
                messages->changeCode(infoThr->numRequest, CODE_MESSAGE_NEW);
                setSnapShotTime(infoThr->numRequest, iow);
                stateReq = STATE_REQ_COMPLATE;

                break;
            case STATE_REQ_COMPLATE:
                messages->changeCode(infoThr->numRequest, CODE_MESSAGE_COMPL);
                return 0;
            }

            std::unique_lock<std::mutex> lk(infoThr->mtx);
            infoThr->condVar.wait(lk, [infoThr, this] (){
                return  this->messages->checkRequest(infoThr->numRequest, CODE_MESSAGE_READY);
            });
            lk.unlock();

        }
        std::cout << "exit thread client" << std::endl;

    }catch(const std::runtime_error& re) {
        std::cerr << "Runtime error: " << re.what() << std::endl;
    } catch(const std::exception& ex) {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    } catch(...) {
        std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
    }
    return 0;
}

int TClient::createRequestStopServer(InfoThread *infoThr, int16_t aCountArgs, char **appchData)
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

void TClient::setSnapShotTime(int16_t numReq, InfoOfWork *iow)
{
    std::lock_guard<std::mutex> lk(mtxDataOfWork);
    infoOfWork.insert(std::pair<int16_t, InfoOfWork* >(numReq, iow));
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
    api[0] = &TServer::call0;
    api[1] = &TServer::call1;
    api[2] = &TServer::call2;
    api[3] = &TServer::call3;
    for(size_t i = 4; i < COUNT_FIELD; i++)
        api[i] = &TServer::call0;

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

        size_t cout_threads = threads.size();
        static size_t max_active_thread = 0;
        if(max_active_thread < cout_threads)
            max_active_thread = cout_threads;
        std::for_each(threads.begin(), threads.end(), [](ThreadInfo2 *thrInfo) {
//            std::cout << "thread.join() " << std::endl;
             thrInfo->thread.join();
        });
        for(size_t i = 0; i < threads.size(); i++)
            delete threads.at(i);
        threads.clear();

//        std::cout << "size threads = " << cout_threads << std::endl;

//        for(size_t i = 0; i < strings.size(); i++) {
//            delete strings[i];
//        }
//        strings.clear();

        delete messages;
        std::cout << "~" << m_sName << "() threads = " << std::dec << cout_threads << " max = " << max_active_thread << std::endl;


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
        thread_count = std::thread::hardware_concurrency();
        thread_count = COUNT_MAX_THREADS;
//        threads.resize(thread_count);
//        for(size_t i = 0; i < thread_count; i++) {
//            threads[i] = std::thread(&TServer::openSession, this, 1);
//        }
//        size_t index_thread = 0;
        ThreadInfo2 *infoThr;
        while(cycle) {
            next = false;
            if(threads.size() < thread_count) {
                infoThr = new ThreadInfo2;
                infoThr->cycle = true;
                infoThr->thread = std::thread(&TServer::openSession, this, infoThr);
                threads.push_back(infoThr);
                continue;
            }



            std::unique_lock<std::mutex> lk(mtx);
            condVar.wait(lk, [this] (){
                return next;
            });
            if(threads.size() > thread_count) {
                threads.back()->cycle = false;
                threads.back()->thread.join();
                delete threads.back();
                threads.pop_back();
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

    lastTaskComplate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


int TClient::worker()
{


    try {

        std::cout << "start " << m_sName << ": " << std::hex << std::this_thread::get_id() << std::endl;

        size_t count_max_thread = 2;
        size_t count_cur_thread = 0;

        int16_t tmp_counter = 1;        // Используется в номере запроса если начинаем с 0, то после обнулении сообщения получаем нулевой запрос
        std::string *strTmp;
        char **ppchTmp = nullptr;
        InfoThread *infoThr;
        while(cycle) {
            int16_t code;
            int16_t req = -1;
            if(messages->getMessageReady(code, req)) {
                switch (code) {
                case CODE_MESSAGE_READY:
        //            std::cout << "notify_one: req = " << req << std::endl;
                    queueThreads[req]->condVar.notify_one();
                    break;
                case CODE_MESSAGE_COMPL:
                    messages->clearMessage(req);
                    delete queueThreads[req];
                    queueThreads.erase(req);
                    break;
                }
            } else {
                messages->getMessageIsEmpty(code, req);

                switch (state) {
                case STATE_START:
//                    strTmp = new std::string("start client");
//                    ppchTmp = new char*[static_cast<size_t>(1)] {    // количество указателей (аргументов)
//                            reinterpret_cast<char*>(strTmp)
//                    };
                    infoThr = new InfoThread;
                    infoThr->numRequest = tmp_counter;
                    infoThr->threadGuard = new scoped_guard(std::thread(&TClient::createRequest, this, infoThr, 1, ppchTmp));       // передаются ссылки
                    queueThreads.insert(std::make_pair(infoThr->numRequest, infoThr));
                    tmp_counter++;
                    state = STATE_WORK;
                    break;
                    //************************************************************************************************
                case STATE_WORK:

//                    strTmp = new std::string("hello world");
//                    ppchTmp = new char*[static_cast<size_t>(1)] {    // количество указателей (аргументов)
//                            reinterpret_cast<char*>(strTmp)
//                    };
                    infoThr = new InfoThread;
                    infoThr->numRequest = tmp_counter;
                    infoThr->threadGuard = new scoped_guard(std::thread(&TClient::createRequest, this, infoThr, 1, ppchTmp));       // передаются ссылки
                    queueThreads.insert(std::make_pair(infoThr->numRequest, infoThr));
                    tmp_counter++;

                    if(tmp_counter >= COUNT_MAX_REQUEST) {
                        state = STATE_COMPLATE_LAUNCH;
                        std::cout << "set state STATE_COMPLATE_LAUNCH" << std::endl;
                    }
                    break;
                    //************************************************************************************************
                case STATE_COMPLATE_LAUNCH:

                    if(queueThreads.empty()) {
                        std::cout << "set state STATE_COMPLATE_PROC" << std::endl;
                        state = STATE_COMPLATE_PROC;
                    }

                    break;
                case STATE_COMPLATE_PROC:
                    strTmp = new std::string("stop server");
                    ppchTmp = new char*[static_cast<size_t>(1)] {    // количество указателей (аргументов)
                            reinterpret_cast<char*>(strTmp)
                    };
                    infoThr = new InfoThread;
                    infoThr->numRequest = tmp_counter;
//                    infoThr->threadGuard = new scoped_guard(std::thread(&TClient::createRequest, this, infoThr, 1, ppchTmp));       // передаются ссылки
                    infoThr->threadGuard = new scoped_guard(std::thread(&TClient::createRequestStopServer, this, infoThr, 1, ppchTmp));       // передаются ссылки
                    queueThreads.insert(std::make_pair(infoThr->numRequest, infoThr));
                    tmp_counter++;
                    state = STATE_COMPLATE_FINISH;
//                    std::cout << "STATE_COMPLATE_FINISH" << std::endl;
                    break;
                case STATE_COMPLATE_FINISH:
                    if(queueThreads.empty()) {
                        cycle = false;
                    }
                    break;
                }

            }


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
//    std::cout << "delete ~" << m_sName << "() queuetThreads" << std::endl;
    try {
//            std::for_each(queueThreads.begin(), queueThreads.end(), [](std::pair<const unsigned long, InfoThread*> &pair) {
//                std::cout << pair.second->threadGuard->getId() << std::endl;
//                pair.second->threadGuard->join(); //
//        //         std::mem_fn(&std::thread::join);
//            });


        for(auto info: queueThreads) {
//            delete info.second->threadGuard;
            delete info.second;
        }
        queueThreads.clear();
        std::cout << "~" << m_sName << "()" << std::endl;
        for(auto info: infoOfWork) {
            delete info.second;
        }

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

void test4() {

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


bool DataMessages::getMessageIsEmpty(int16_t &code, int16_t &req)
{
    try {
        code = -1;
        req = -1;
        std::lock_guard<std::mutex> lk(mtx);

        for(size_t i = 0; i < data->size(); i++) {
            code = *reinterpret_cast<int16_t*>(&data->at(i)[0]);
            req = *reinterpret_cast<int16_t*>(&data->at(i)[2]);
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
        req = *reinterpret_cast<int16_t*>(&data->at(i)[2]);
        switch (code) {
        case CODE_MESSAGE_COMPL:
        case CODE_MESSAGE_READY:
            return true;
        }
    }
    return false;
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
    return false;
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

bool DataMessages::changePointer(int16_t numReq, char **aPointer)
{
    std::lock_guard<std::mutex> lk(mtx);
    for(size_t i = 0; i < data->size(); i++) {
        int16_t check_request = *reinterpret_cast<int16_t*>(&data->at(i)[2]);
        if(check_request == numReq) {
            memcpy(data->at(i).data() + SIZE_PARAMETERS_INT, &aPointer, sizeof aPointer);
            return true;
        }
    }
    return false;
}

bool DataMessages::changeApiFunc(int16_t numReq, int16_t numApi)
{
    try{
        int16_t index;
        getIndexOfReqNum(numReq, index);
        std::lock_guard<std::mutex> lk(mtx);
        memcpy(data->at(static_cast<size_t>(index)).data() + (sizeof numApi)*3, &numApi, sizeof numApi);
        return true;
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

bool DataMessages::changeCode(int16_t numReq, int16_t change_code)
{
    try {
        std::lock_guard<std::mutex> lk(mtx);
        for(size_t i = 0; i < data->size(); i++) {
            int16_t check_request = *reinterpret_cast<int16_t*>(&data->at(i)[2]);
            if(check_request == numReq) {
                memcpy(data->at(static_cast<size_t>(i)).data(), &change_code, sizeof change_code);
                return true;
            }
        }
    } catch (...) {
        std::cout << "exception checkRequest" << std::endl;;
    }
    return false;
}



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
