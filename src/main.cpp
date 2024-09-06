#include <iostream>

#include "RFModule.hpp"
#include "userLayer.hpp"
#include <memory>

using Func = std::function<void()>;

void ExeFunc(Func func){
    if(func){
        func();
    }else{
        std::cout << "No function to execute" << std::endl;
    }
}
int main()
{
    UserSDKImpl userSDK;

    // userSDK.Disconnect().SetFrequency(3.7).Load().Execute();
    // std::cout << "+++++++++++++++++++++++" << std::endl;


    ExeFunc(nullptr);
    

    return 0;
}