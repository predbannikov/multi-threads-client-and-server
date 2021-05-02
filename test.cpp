#include "test.h"
#include <iterator>
#include <string.h>
#include <iomanip>
#include <algorithm>
#include "sha256.h"


TAbstract::~TAbstract() {
    if(m_sName == "server") {
        std::cout << std::endl << "Data" << std::hex << std::endl;
        for(auto item: *data) {
                for(auto i: item)
                    std::cout << (0xFF&i) << " ";
                std::cout << std::endl;
        }
    }

}

int TAbstract::openSession(size_t anNumMessage)
{


    std::cout << std::endl;
    std::cout << "Server " << std::endl;
//    for(auto i: data->at(anNumMessage))
//        std::cout << (0xFF&i) << " ";
//    std::cout << std::endl;
    int64_t intPart = *reinterpret_cast<int64_t*>(&data->at(anNumMessage).at(0));
    int16_t codeMessage = static_cast<int16_t>(intPart);
    //std::cout << "code: " << codeMessage << std::endl;
    int16_t funcNum = static_cast<int16_t>(intPart >> 16);
    intPart >>= 16;
    int16_t countArgs = static_cast<int16_t>(intPart);
    void (TAbstract::*pFunc)(int16_t&, char**);
    pFunc = api[funcNum];

    //std::cout << "SIZE_PARAMETERS_INT " << SIZE_PARAMETERS_INT << std::endl;
//    char **ptr = reinterpret_cast<char**>(*reinterpret_cast<char**>(&data->at(anNumMessage).at(SIZE_PARAMETERS_INT)));
//    long mark = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//    for(long i = 0; i < 500000000; i++) {
//        char **pTmp = reinterpret_cast<char**>(*reinterpret_cast<char**>(&data->at(anNumMessage).at(SIZE_PARAMETERS_INT)));
//    }
//    long dmark = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - mark;
//    std::cout << "1 dmark = " << std::dec << dmark << std::endl;


    char& p = data->at(anNumMessage).at(SIZE_PARAMETERS_INT);
    char** ptr = nullptr;
    memcpy(&ptr, &p, sizeof(char*));
    //std::cout << ptr << std::endl;
    (this->*pFunc)(countArgs, ptr);
    return 0;
}

void TAbstract::call(int16_t &nArgs, char **ppchArgs){

    std::string *strtmp = reinterpret_cast<std::string*>(*ppchArgs);
    std::cout << *strtmp << " " << std::this_thread::get_id() << std::endl;
    std::string *strResponse = new std::string(sha256(strtmp));
//    std::cout << *strResponse << std::endl;;
    delete ppchArgs;
    nArgs++;
    ppchArgs = new char*[static_cast<size_t>(nArgs)] {
            reinterpret_cast<char*>(strtmp),
            reinterpret_cast<char*>(strResponse)
    };

//    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

int TAbstract::createRequest(InfoThread *infoThr, int16_t aCountArgs, char **appchData)
{
    //std::cout << "bt" << SIZE_CODE_MESSAGE << " " << SIZE_NUM_FUNC_API << std::endl;
    //std::lock_guard<std::mutex> lock(mtx);
    std::cout << "createRequest: " << std::hex << std::this_thread::get_id() << std::endl;

    const int16_t numApiFunc = 1;
    const int16_t codeMessage = CODE_MESSAGE_NEW ;//MESSAGE_FILLED;
    const int16_t countArgs = aCountArgs;
    int64_t prepFuncArgs = 0;
    prepFuncArgs <<= sizeof (int16_t) * 8;      // Пустое место
    prepFuncArgs |= countArgs;
    prepFuncArgs <<= 16;
    prepFuncArgs |= numApiFunc;
    prepFuncArgs <<= SIZE_NUM_FUNC_API * 8;
    prepFuncArgs |= codeMessage;

//    const size_t indexTopData = getIndexBlockReady();
    //std::cout << "index block ready = " << std::hex << indexTopData << std::endl;
    memcpy(data->at(infoThr->indxMessage).data(), &prepFuncArgs, SIZE_PARAMETERS_INT);  // функция + RET + ARGS
//    char **ppchTmp = new char*[static_cast<size_t>(1)] {
//            reinterpret_cast<char*>(new size_t(anNumReq))
//    };
    memcpy(data->at(infoThr->indxMessage).data() + SIZE_PARAMETERS_INT, &appchData, sizeof appchData);

//    std::cout << std::endl << "Client" << std::hex << std::endl;
//    for(auto i: data->at(indexTopData))
//        std::cout << (0xFF&i) << " ";
//    std::cout << std::endl;
//    std::this_thread::sleep_for(std::chrono::milliseconds(1050));
//    std::cout << "end creat request befor" << std::endl;

//    std::unique_lock<std::mutex> lk(infoThr->mtx);
//    std::vector<char> &ptr = data->at(infoThr->indxMessage);
//    infoThr->condVar.wait(lk, [ptr] (){
//        return ptr[0] == static_cast<char>(0xEE);
//    });

//    std::cout << "end creat request" << std::endl;
    return 0;
}

size_t TAbstract::getIndexBlockReady()       // Поиск блока для клиента
{
    for(size_t i = 0; i < data->size(); i++) {
        if(*reinterpret_cast<int16_t*>(&data->at(i).at(0)) == CODE_MESSAGE_EMPTY) {     // Код свободного блока сообщения
            return i;
        }
    }
    // Только если в очереди нет свободных блоков
    std::unique_lock<std::mutex> lk(getMtx);
    lk.lock();
    data->push_back(std::vector<char>(SIZE_BLOCK_MESSAGE, 0));
    lk.unlock();
//    std::cout << "data = " << data << std::endl;
    return data->size()-1;
}

long TAbstract::getIndexBlockFilled()       // Поиск блока для сервера
{
    std::unique_lock<std::mutex> lock(getMtx);
    for(size_t i = 0; i < data->size(); i++) {
//        std::cout << "code = " << std::hex << *reinterpret_cast<int16_t*>(&data->at(i).at(0)) << std::endl;
//        std::cout << " getIndexBlockFilled " << (short)data->at(i)[0] << (short)data->at(i)[1] << std::endl;
//        std::cout << "i = " << i  << " " << data->size() << " " << data << std::endl;
//        try {
//            if(data->at(i).at(0) == 0) {
//                std::cout << "data = " << data << std::endl;
//            }
//        } catch (...) {
//            std::cout << "stop " << data->size() << std::endl;
//        }
//        if(data->at(i)[0] == 0) {
//            std::cout << "data = " << data << std::endl;
//        }
        int16_t tmp = *reinterpret_cast<int16_t*>(&data->at(i)[0]);
        if(tmp == CODE_MESSAGE_NEW) {
//            std::cout << "lock_guard i= " << i << " " << std::this_thread::get_id() << std::endl;
            data->at(i).at(0) = static_cast<char>(0x55);
//            std::cout << "code = " << std::hex << *reinterpret_cast<int16_t*>(&data->at(i).at(0));

            return static_cast<long>(i);
        }
    }
    return -1;
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
TServer::TServer(std::vector<std::vector<char > > *data_) : TAbstract(data_) {
    m_sName = "server";
}

int TServer::writeMessage() {
    std::cout << "Server write" << std::endl;
    return 0;
}

int TServer::readMessage() {
    return 0;
}

TServer::~TServer() {
    std::cout << "~Sever()" << std::endl;
}

int TServer::worker()
{
    long tmp_counter = 0;
    //std::shared_ptr<scoped_guard> scope_guard_share;
    while(true) {
        const long indexData = getIndexBlockFilled();
        if(indexData < 0) {
//            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        // Дальше проходим только если в очереди есть есть необработанные блоки

        new scoped_guard(std::thread(&TServer::openSession, this, indexData));     // TODO решить что то с утечкой памяти


//        if(tmp_counter%10 == 10)
//            std::cout << this << std::endl;
//        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if(tmp_counter >= 10)
            break;
        tmp_counter++;
    }
    std::cout << "\nServer worker end" << std::endl;
    return 0;
}


/************************************************
 *                  Client
 * **********************************************/
TClient::TClient(std::vector<std::vector<char > > *data_) : TAbstract(data_) {
    m_sName = "client";
}


int TClient::worker()
{
    size_t tmp_counter = 0;
    while(true) {


        std::string *strTmp = new std::string("hello world");
        char **ppchTmp = new char*[static_cast<size_t>(1)] {    // количество указателей (аргументов)
                reinterpret_cast<char*>(strTmp)
        };

        //thread_guard thr_req(std::thread(&TClient::createRequest, this, tmp_counter, 1, ppchTmp));  // 1 количество аргументов
//        std::thread thr(&TClient::createRequest, this, tmp_counter, 1, ppchTmp);
//        thr.detach();
        InfoThread *infoThr = new InfoThread;
        infoThr->indxMessage = getIndexBlockReady();
        infoThr->scGuard = new scoped_guard(std::thread(&TClient::createRequest, this, infoThr, 1, ppchTmp));
        queueThreads.insert(std::make_pair(infoThr->indxMessage, infoThr));

        if(tmp_counter%10 == 10)
            std::cout << this << std::endl;
//        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if(tmp_counter >= 10)
            break;
        tmp_counter++;
    }
    std::cout << "\nClient worker end" << std::endl;
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
    std::cout << "~Client()" << std::endl;
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
            //std::cout << "data " << vec2.data() << " " << vec2.size() << std::endl;
            int16_t tmp = *reinterpret_cast<int16_t*>(&vec2.at(0).at(0));

        }catch(const std::runtime_error& re)
            {
                // speciffic handling for runtime_error
                std::cerr << "Runtime error: " << re.what() << std::endl;
            }
            catch(const std::exception& ex)
            {
                // speciffic handling for all exceptions extending std::exception, except
                // std::runtime_error which is handled explicitly
                std::cerr << "Error occurred: " << ex.what() << std::endl;
            }
            catch(...)
            {
                // catch any other errors (that we have no information about)
                std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
            }
        long mark2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        long dmark = mark2 - mark;
        if(dmark > 5)
            break;
    }
    std::cout << "stop test" << std::endl;
}

void test() {


    std::vector<std::vector<char >> *vec = new std::vector<std::vector<char >>(0, std::vector<char>(SIZE_BLOCK_MESSAGE));

//    stack[0] = 1;
//    std::cout << stack[0] << std::endl;
//    std::cout << std::endl;

    TServer server(vec);
    TClient client(vec);

    long hardware_threads = std::thread::hardware_concurrency();
    std::cout << hardware_threads << std::endl;

    scoped_guard thrClient(std::thread(&TClient::worker, &client));
    scoped_guard thrServer(std::thread(&TServer::worker, &server));

//    std::this_thread::sleep_for(std::chrono::milliseconds(100));

//    hardware_threads = std::thread::hardware_concurrency();
//    std::cout << hardware_threads << std::endl;
    //thrClient.join();
    //delete client;

//    std::cout << "do something in current thread" << std::endl;

}

size_t DataMessages::getIndexBlockReady()
{
    for(size_t i = 0; i < data->size(); i++) {
        if(*reinterpret_cast<int16_t*>(&data->at(i).at(0)) == CODE_MESSAGE_EMPTY) {     // Код свободного блока сообщения
            return i;
        }
    }
    // Только если в очереди нет свободных блоков
    std::unique_lock<std::mutex> lk(mtx);
    lk.lock();
    data->push_back(std::vector<char>(SIZE_BLOCK_MESSAGE, 0));
    lk.unlock();
//    std::cout << "data = " << data << std::endl;
    return data->size()-1;
}

long DataMessages::getIndexBlockFilled()
{
    std::unique_lock<std::mutex> lock(mtx);
     for(size_t i = 0; i < data->size(); i++) {
 //        std::cout << "code = " << std::hex << *reinterpret_cast<int16_t*>(&data->at(i).at(0)) << std::endl;
 //        std::cout << " getIndexBlockFilled " << (short)data->at(i)[0] << (short)data->at(i)[1] << std::endl;
 //        std::cout << "i = " << i  << " " << data->size() << " " << data << std::endl;
 //        try {
 //            if(data->at(i).at(0) == 0) {
 //                std::cout << "data = " << data << std::endl;
 //            }
 //        } catch (...) {
 //            std::cout << "stop " << data->size() << std::endl;
 //        }
 //        if(data->at(i)[0] == 0) {
 //            std::cout << "data = " << data << std::endl;
 //        }
         int16_t tmp = *reinterpret_cast<int16_t*>(&data->at(i)[0]);
         if(tmp == CODE_MESSAGE_NEW) {
 //            std::cout << "lock_guard i= " << i << " " << std::this_thread::get_id() << std::endl;
             data->at(i).at(0) = static_cast<char>(0x55);
 //            std::cout << "code = " << std::hex << *reinterpret_cast<int16_t*>(&data->at(i).at(0));

             return static_cast<long>(i);
         }
     }
     return -1;
}
