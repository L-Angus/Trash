#ifndef USERLAYER_HPP
#define USERLAYER_HPP

#include <string>
#include <memory>
#include <map>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#include "Buffer.hpp"
#include "RPCServer.hpp"
#include "RPCHandlers.hpp"

#include <map>


using SerializeAction = std::function<void(STREAM_IN&)>;

struct Task{
    unsigned int TaskID;
    SerializeAction Action;
};


template<typename RFTYPE>
struct RFWrapper{
    public:
    void EnqueueTask(Task&& task){
        static_cast<RFTYPE*>(this)->enqueue(std::move(task));
    }
    void ExecuteTasks(){
        static_cast<RFTYPE*>(this)->execute();
    }
    void ClearTasks(){
        static_cast<RFTYPE*>(this)->clear();
    }

    protected:
    virtual void enqueue(Task&& task) = 0;
    virtual void execute() = 0;
    virtual void clear() = 0;
};

struct SDKWrapper{
    std::map<unsigned int, SerializeAction> tasks;

    struct SerilaizeExecutor {
        SDKWrapper& mWrapper;
        SerilaizeExecutor(SDKWrapper& wrapper) : mWrapper(wrapper){}
        void operator()(){
            STREAM_IN InStream;
            for(const auto& [taskID, taskAction] : mWrapper.tasks){
                taskAction(InStream);
            }
        }
    };

    void EnqueueTask(Task&& task){
        tasks[task.TaskID] = task.Action;
    }

    void ExecuteTasks() {
        SerilaizeExecutor(*this)();
    }

    void ClearTasks() {
        tasks.clear();
    }
};


class OnessdkImpl{
    public:
    OnessdkImpl() : m_wrapper(std::make_unique<SDKWrapper>()){};
    ~OnessdkImpl(){};

    void Load(){
        m_wrapper->EnqueueTask({1, [](STREAM_IN& inStream){
            std::cout << "Task 1" << std::endl;
        }});
    }

    void SetType(const std::string& type){
        auto handler = RpcHandler::GetHandler(type);
        m_wrapper->EnqueueTask({6, [handler](STREAM_IN& inStream){
            STREAM_OUT outStream;
            if(handler){
                CHK_RET ret = handler(inStream, outStream);
                std::cout << "RpcHandler Results: "<< ret << std::endl;
            }
        }});
            
    }
    void Connect(){
        m_wrapper->EnqueueTask({4, [](STREAM_IN& inStream){
            std::cout << "Task 4" << std::endl;
        }});
    }
    void Disconnect(){
        m_wrapper->EnqueueTask({5, [](STREAM_IN& inStream){
            std::cout << "Task 5" << std::endl;
        }});
    }
    void Execute(){
        m_wrapper->ExecuteTasks();
        m_wrapper->ClearTasks();
    }
    void SetFrequency(double freq){
        m_wrapper->EnqueueTask({2, [freq](STREAM_IN& inStream){
            std::cout << "Task 2" << std::endl;
            std::map<double, unsigned int> freqSet;
            freqSet[freq] = 1;
            inStream << freqSet;
        }});
    }
    void SetPower(double power){
        m_wrapper->EnqueueTask({3, [power](STREAM_IN& inStream){
            std::cout << "Task 3" << std::endl;
            std::map<double, unsigned int> powerSet;
            powerSet[power] = 1;
            inStream << powerSet;
        }});
    }


    private:
    std::unique_ptr<SDKWrapper> m_wrapper = nullptr;
};

class UserSDKImpl{
    public:
    UserSDKImpl() : m_onessdk(std::make_unique<OnessdkImpl>()){
        m_onessdk->SetType("CW");
    };
    ~UserSDKImpl(){};

    UserSDKImpl& Load(){
        m_onessdk->Load();
        return *this;
    }
    UserSDKImpl& Connect(){
        m_onessdk->Connect();
        return *this;
    }
    UserSDKImpl& Disconnect(){
        m_onessdk->Disconnect();
        return *this;
    }
    UserSDKImpl& Execute(){
        m_onessdk->Execute();
        return *this;
    }
    UserSDKImpl& SetFrequency(double freq){
        m_onessdk->SetFrequency(freq);
        return *this;
    }
    UserSDKImpl& SetPower(double power){
        m_onessdk->SetPower(power);
        return *this;
    }

    private:
    std::unique_ptr<OnessdkImpl> m_onessdk = nullptr;

};

#endif