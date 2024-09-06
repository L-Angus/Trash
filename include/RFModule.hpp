#ifndef RFMODULE_HPP
#define RFMODULE_HPP

#include <iostream>

template <typename RFModuleType>
struct RFModuleBuilder{
public:
    RFModuleType& Load(){
        static_cast<RFModuleType*>(this)->loadImpl();
        return this->get();
    }

    RFModuleType& Connect(){
        static_cast<RFModuleType*>(this)->connectImpl();
        return this->get();
    }
protected:
    virtual void loadImpl() = 0;
    virtual void connectImpl() = 0;

private:
    RFModuleType& get(){
        return static_cast<RFModuleType&>(*this);
    }
};   

class CWModule : public RFModuleBuilder<CWModule>{
public:
    CWModule(const std::string& pin){
        std::cout << "CW Module Created" << std::endl;
    }
    void loadImpl() override{
        std::cout << "CW Module Loaded" << std::endl;
    }

    void connectImpl() override{
        std::cout << "CW Module Connected" << std::endl;
    }
};

class DTModule : public RFModuleBuilder<DTModule>{
public:
    DTModule(const std::string& pin){
        std::cout << "DT Module Created" << std::endl;
    }
    void loadImpl() override{
        std::cout << "DT Module Loaded" << std::endl;
    }

    void connectImpl() override{
        std::cout << "DT Module Connected" << std::endl;
    }
};

class MODModule : public RFModuleBuilder<MODModule>{
public:
    MODModule(const std::string& pin){
        std::cout << "MOD Module Created" << std::endl;
    }
    void loadImpl() override{
        std::cout << "MOD Module Loaded" << std::endl;
    }

    void connectImpl() override{
        std::cout << "MOD Module Connected" << std::endl;
    }
};



#endif