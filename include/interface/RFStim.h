#ifndef RF_STIM_H
#define RF_STIM_H

#include <string>

class RF_STIM_DEF;
class NRFStim{
    public:
    NRFStim();
    ~NRFStim();

    RF_STIM_DEF& Config(const std::string& stimName);
};

#endif