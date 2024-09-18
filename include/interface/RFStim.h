#ifndef RF_STIM_H
#define RF_STIM_H

#include <memory>
#include <string>
#include <vector>

#include ""
#include "impl/StimDefInner.h"

class RF_STIM_DEF;
class NRFStimPri;

class NRFStim {
public:
  NRFStim();
  ~NRFStim();

  RF_STIM_DEF &Config(const std::string &stimName) {}

protected:
  void Restore();
  void Cleanup();

private:
  std::shared_ptr<NRFStimPri> m_pri = nullptr;
};

class RFStimImpl;
class RF_STIM_DEF {
  friend class NRFStimPri;

public:
  explicit RF_STIM_DEF(const std::vector<std::string> &stimDefs, const std::string &flist);
  ~RF_STIM_DEF();

  RF_STIM_DEF &Load();
  RF_STIM_DEF &Connect();
  RF_STIM_DEF &Disconnect();
  RF_STIM_DEF &Execute();
  RF_STIM_DEF &SetFreqListIndex(size_t index);
  RF_STIM_DEF &SetFreqListIndex(const std::vector<size_t> &indexs);
  RF_STIM_DEF &SetFreqListName(const std::string &name);
  RF_STIM_DEF &SetPower(double power);
  RF_STIM_DEF &SetPowerList(const std::vector<double> &powers);
  RF_STIM_DEF &SetRepeatCount(size_t repeat);

protected:
  void ReLoadStim();

private:
  std::shared_ptr<StimDefInner> m_stim = nullptr;
  std::shared_ptr<RFStimImpl> m_impl = nullptr;
  std::shared_ptr<WaveFileImpl> m_wave = nullptr;
  bool mIsLoaded{false};
};

#endif