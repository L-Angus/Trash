#ifndef USERLAYER_HPP
#define USERLAYER_HPP

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <sstream>

using STREAM_IN = std::ostringstream;
void RpcTool(STREAM_IN& InStream) {
    std::string input = InStream.str();
    std::cout << "RPC called with InStream: \n";
    std::cout << input << std::endl;
}

struct TaskData{
    std::vector<unsigned int> ports;
    std::vector<double> freqs;
    double power;
    unsigned int repeatcount;
};



typedef std::function<void(STREAM_IN&)> Task;

struct SDKWrapper{
    
    std::unordered_map<std::string, Task> tasks;
    STREAM_IN InStream;
    std::vector<std::string> predefinedTaskOrder = {
        "Load",
        "SetFrequency",
        "SetPower",
        "Connect",
        "Disconnect"
    };

    struct SerilaizeExecutor {

        SDKWrapper& mWrapper;
        SerilaizeExecutor(SDKWrapper& wrapper) : mWrapper(wrapper){}
        void operator()(){
            // control execute order
            for (const auto& taskName : mWrapper.predefinedTaskOrder) {
                if (mWrapper.tasks.count(taskName)) {
                    mWrapper.tasks[taskName](mWrapper.InStream);
                }
            }
            RpcTool(mWrapper.InStream);
        }
    };

    void EnqueueTask(const std::string& taskName, Task&& task){
        tasks[taskName] = std::move(task);
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
        m_wrapper->EnqueueTask("Load", [this](STREAM_IN& in){
            in << "Load Executed in OnessdkImpl"<< std::endl;
        });
    }
    void Connect(){
        m_wrapper->EnqueueTask("Connect", [this](STREAM_IN& in){
            in << "Connect Executed in OnessdkImpl"<< std::endl;
        });
    }
    void Disconnect(){
        m_wrapper->EnqueueTask("Disconnect", [this](STREAM_IN& in){
            in << "Disconnect Executed in OnessdkImpl" << std::endl;
        });
    }
    void Execute(){
        m_wrapper->ExecuteTasks();
        m_wrapper->ClearTasks();
    }
    void SetFrequency(double freq){
        m_wrapper->EnqueueTask("SetFrequency", [this, freq](STREAM_IN& in){
            in << "SetFrequency Executed in OnessdkImpl: "<< freq << std::endl;
        });
    }
    void SetPower(double power){
        m_wrapper->EnqueueTask("SetPower", [this, power](STREAM_IN& in){
            in << "SetPower Executed in OnessdkImpl: "<< power << std::endl;
        });
    }


    private:
    std::unique_ptr<SDKWrapper> m_wrapper = nullptr;
};

class UserSDKImpl{
    public:
    UserSDKImpl() : m_onessdk(std::make_unique<OnessdkImpl>()){};
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