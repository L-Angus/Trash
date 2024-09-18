#include "interface/RFStim.h"
#include "kits/utils.hpp"

#include <unordered_map>

using RFStimMap = std::unordered_map<std::string, RF_STIM_DEF>;

class NRFStimPri {
public:
  RFStimMap mStimMap;
  void Cleanup() { RFStimMap().swap(mStimMap); }
  void ReLoadStim() {
    for (auto &[StimName, StimObject] : mStimMap) {
      StimObject.ReLoadStim();
    }
  }
};
NRFStim::NRFStim() : m_pri(std::make_shared<NRFStimPri>()) {}

NRFStim::~NRFStim() { m_pri->Cleanup(); }

RF_STIM_DEF &NRFStim::Config(const std::string &stimName) {
  auto stimFile = SettingManager::GetInstance().GetPropOf("Resource", "StimFile");
  auto stimResult = RFUTILS::DoQuery(stimFile, stimName);
  if (stimResult.empty())
    throw std::runtime_error("Not find such stimName: " + stimName);

  if (m_pri->mStimMap.find(stimName) == m_pri->mStimMap.end()) {
    auto flistFile = SettingManager::GetInstance().GetPropOf("Resource", "FreqListFile");
    m_pri->mStimMap.emplace(stimName, RF_STIM_DEF{stimResult, flistFile});
  }

  return m_pri->mStimMap.at(stimName);
}

void NRFStim::Restore() { m_pri->ReLoadStim(); }
void NRFStim::Cleanup() { m_pri->Cleanup(); }

RF_STIM_DEF::RF_STIM_DEF(const std::vector<std::string> &stimDefs, const std::string &flist)
    : m_stim(std::make_shared<StimDefInner>(stimDefs)),
      m_impl(std::make_shared<RFStimImpl>(m_stim->Type(), m_stim->Pin())) {
  m_stim->SetFreqListFile(flist);
}

RF_STIM_DEF::~RF_STIM_DEF() { mIsLoaded = false; }
RF_STIM_DEF &RF_STIM_DEF::Load() {
  m_stim->UpdateFreqListValuesByName(m_stim->FreqListName());
  m_impl->SetDefaultSettings(m_stim->Type(), m_stim->Frequencies(), m_stim->Power());
  if (m_stim->Type() == "MOD") {
    m_wave = std::make_shared<WaveFileImpl>(m_stim->WaveFile());
    m_wave->LoadWaveToDDR();
  }
  mIsLoaded = true;
  return *this;
}

RF_STIM_DEF &RF_STIM_DEF::Connect() {
  if (!mIsLoaded) {
    throw std::runtime_error("Please load stim first!");
  }
  m_impl->Connect();
  return *this;
}

RF_STIM_DEF &RF_STIM_DEF::Disconnect() {
  if (!mIsLoaded)
    throw std::runtime_error("Please load stim first!");
  m_impl->Disconnect();
  return *this;
}

RF_STIM_DEF &RF_STIM_DEF::Execute() {
  if (!mIsLoaded)
    throw std::runtime_error("Please load stim first!");
  m_impl->Execute();
  return *this;
}

RF_STIM_DEF &RF_STIM_DEF::SetFreqListIndex(size_t index) {
  if (!mIsLoaded)
    throw std::runtime_error("Please load stim first!");
  m_impl->SetFrequency(m_stim->GetFrequencyByIndex(index));
  return *this;
}

RF_STIM_DEF &RF_STIM_DEF::SetFreqListIndex(const std::vector<size_t> &indexs) {
  if (!mIsLoaded)
    throw std::runtime_error("Please load stim first!");
  m_impl->SetFrequency(m_stim->GetFrequencyListByIndex(indexs));
  return *this;
}

RF_STIM_DEF &RF_STIM_DEF::SetFreqListName(const std::string &name) {
  if (!mIsLoaded)
    throw std::runtime_error("Please load stim first!");
  m_stim->UpdateFreqListValuesByName(name);
  m_stim->Type() == "DT" ? SetFreqListIndex({0, 1}) : SetFreqListIndex(0);
  return *this;
}

RF_STIM_DEF &RF_STIM_DEF::SetPower(double power) {
  if (!mIsLoaded)
    throw std::runtime_error("Please load stim first!");
  m_impl->SetPower(power);
  return *this;
}

RF_STIM_DEF &RF_STIM_DEF::SetPowerList(const std::vector<double> &powers) {
  if (!mIsLoaded)
    throw std::runtime_error("Please load stim first!");
  m_impl->SetPowerList(powers);
  return *this;
}

RF_STIM_DEF &RF_STIM_DEF::SetRepeatCount(size_t repeat) {
  if (!mIsLoaded)
    throw std::runtime_error("Please load stim first!");
  m_impl->SetRepeatCount(repeat);
  return *this;
}