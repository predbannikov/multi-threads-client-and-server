#ifndef TEST_H
#define TEST_H

#include <iostream>
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <chrono>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <map>
#include <atomic>


class TAbstract;

#define COUNT_FIELD             10                                          // Колличества функций
#define SIZE_PARAMETERS_INT     sizeof (int64_t)
#define SIZE_CODE_MESSAGE       sizeof (int16_t)
#define SIZE_NUM_FUNC_API       sizeof (int16_t)
#define SIZE_NARGS              sizeof (int16_t)
#define SIZE_PDATA              sizeof(char**)
#define SIZE_BLOCK_MESSAGE      SIZE_PARAMETERS_INT+SIZE_PDATA
#define SIZE_API_TABLE          SIZE_BLOCK_MESSAGE*COUNT_FIELD
#define MESSAGE_FILLED          0x01
#define MESSAGE_PREPARED        0x02
#define MESSAGE_COMPLATE        0x03

#define CODE_MESSAGE_EMPTY      0x0000
#define CODE_MESSAGE_PREP       0x5F55
//#define CODE_MESSAGE_PROC       0x5555
#define CODE_MESSAGE_NEW        0x5F5F
#define CODE_MESSAGE_READY      0x55FF
#define CODE_MESSAGE_COMPL      0x0055

#define GET_N_ARGS_FROM_MEM(a) \
                b =  0xFF&*(a+3+SIZE_FUNC_API);     b <<= 8; \
                b |= 0xFF&*(a+2+SIZE_FUNC_API);     b <<= 8; \
                b |= 0xFF&*(a+1+SIZE_FUNC_API);     b <<= 8; \
                b |= 0xFF&*(a+SIZE_FUNC_API); \
                return b;


#define GET_P_FUNK_FROM_MEM(a) \
                b = 0xFF&*(a+3);     b <<= 8; \
                b |= 0xFF&*(a+2);     b <<= 8; \
                b |= 0xFF&*(a+1);     b <<= 8; \
                b |= 0xFF&*a; \
                return b;

#define GET_P_DADA_FROM_MEM(a, b) \
                b =  0xFF&*(a+7+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+6+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+5+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+4+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+3+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+2+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+1+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+SIZE_FUNC_API+sizeof(int));


void test();

struct func
{
    int &i;
    func(int& i_) : i(i_){}
    void operator()()
    {
        std::cout << i << std::endl;
        for(unsigned j=0;j<100000;++j)
        {
            i |= 0xFFFFFFFF;

        }
        std::cout << i << std::endl;
    }
};

class scoped_guard
{
    std::thread t;
public:
    std::thread::id getId() {
        return t.get_id();
    }
    explicit scoped_guard(std::thread t_) : t(std::move(t_)) {
        if(!t.joinable())
            throw std::logic_error("No thread");
    }
    ~scoped_guard()
    {
        std::cout << "end thread: " << std::hex << std::this_thread::get_id() << std::endl;
        t.join();
    }
    scoped_guard(scoped_guard const&) = delete;
    scoped_guard& operator=(scoped_guard const&) = delete;
};

struct InfoThread {
//    std::thread t;

public:
//    explicit InfoThread(size_t size_, std::thread t_) : thread(std::move(t_)) {
//        indxMessage = size_;
//        if(!thread.joinable())
//            throw std::logic_error("No thread");
//    }
    ~InfoThread() { delete threadGuard; }
    std::condition_variable condVar;
    std::mutex mtx;
    scoped_guard *threadGuard;
    int16_t numRequest;
    int complate = 0;
};

struct DataMessages {
    DataMessages() {data = new std::vector<std::vector<char >>(0, std::vector<char>(SIZE_BLOCK_MESSAGE));}
    ~DataMessages() { delete data;
                    std::cout << "vector deleted" << std::endl;
                    }
    std::vector<std::vector<char > > *data;
    size_t getIndexBlockReady();
    long getIndexBlockFilled();
    bool messagesIsFilled();
    bool checkRequest(int16_t numReq, int16_t check_code);
//    int16_t getCodeQueue();
    bool getMessageInfo(int16_t &code, int16_t &req);
    void getIndexOfReqNum(int16_t req, int16_t &index);
    bool setCodeOfReqNum(int16_t req, int16_t code);

    bool changeData(char *ptrData, int16_t req, int16_t code_);
    void addMessage(char *ptrData);
    bool readMessage(char *&ptrData);
    std::mutex mtx;

public:
    bool clearMessage(int16_t req);
private:
};

class TAbstract
{
    std::mutex getMtx;
public:
    DataMessages *messages;
    bool cycle = true;
    std::string m_sName;
    TAbstract(DataMessages *data_) : messages(data_) {
    }
    virtual int writeMessage() = 0;
    virtual int readMessage() = 0;
    virtual int worker() = 0;
    virtual ~TAbstract();
    int createRequest(InfoThread *infoThr, int16_t aCountArgs, char **appchData);
    int createRequestStopServer(InfoThread *infoThr, int16_t aCountArgs, char **appchData);

//    size_t getIndexBlockReady();
//    long getIndexBlockFilled();


    TAbstract(const TAbstract&) = delete;
    TAbstract& operator=(const TAbstract&) = delete;

    std::mutex mtx;
};

class TServer : public TAbstract
{
    void (TServer::*api[COUNT_FIELD])(int16_t&, char**);
    std::thread t;
    std::vector<std::thread> threads;
    std::vector<std::string*> strings;
public:
    int openSession(size_t anNumMessage);
    void call(int16_t &, char**);

    TServer(DataMessages *messages_);
    int worker() override;
    int writeMessage() override;
    int readMessage() override;
    ~TServer() override;
    TServer(TServer const&) = delete;
    TServer& operator=(TServer const&) = delete;
};

class TClient : public TAbstract
{
    std::map<int16_t, InfoThread* > queueThreads;
    std::thread t;
public:
    TClient(DataMessages *messages_);
    int worker() override;
    int writeMessage() override;
    int readMessage() override;
    ~TClient() override;
    TClient(TClient const&) = delete;
    TClient& operator=(TClient const&) = delete;
};


#endif // TEST_H
