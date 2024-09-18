#include <iostream>

#include "impl/FreqListInner.h"
#include "impl/RFConfigManager.h"
#include "kits/csvparser.hpp"
#include "stim/RFModule.hpp"
#include "stim/userLayer.hpp"

#include <memory>

using Func = std::function<void()>;

void ExeFunc(Func func) {
  if (func) {
    func();
  } else {
    std::cout << "No function to execute" << std::endl;
  }
}
int main() {
  UserSDKImpl userSDK;

  userSDK.Disconnect().SetFrequency(3.7).SetPower(10).Load().Execute();
  userSDK.Disconnect().SetPower(100).SetFrequency(66).Load().Execute();
  std::cout << "+++++++++++++++++++++++" << std::endl;

  return 0;
}