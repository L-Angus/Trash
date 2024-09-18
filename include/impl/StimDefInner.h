#ifndef STIM_DEF_INNER_H
#define STIM_DEF_INNER_H

#include <memory>
#include <string>
#include <vector>

#include "impl/FreqListInner.h"
#include "kits/utils.hpp"

struct StimConfiguration {
  std::string stimName;
  std::string stimType;
  std::string triggerType;
  std::string pinName;
  std::string freqListName;
  std::vector<size_t> freqListIndexs;
  std::vector<double> freqs;
  std::vector<double> powers;
  size_t repeatCount = 0;
  std::string waveFile;
};

class StimDefInner {
public:
  explicit StimDefInner(const std::vector<std::string> &stimDefs)
      : mStimDefs(stimDefs), mFreqList(std::make_shared<FreqListInner>(stimDefs.at(4), stimDefs.at(5))) {
    mStimConfig.stimName = stimDefs.at(0);
    mStimConfig.stimType = stimDefs.at(1);
    mStimConfig.triggerType = stimDefs.at(2);
    mStimConfig.pinName = stimDefs.at(3);
    mStimConfig.freqListName = mFreqList->GetFreqListName();
    mStimConfig.freqListIndexs = mFreqList->GetFreqListIndexs();
    mStimConfig.powers = ParsePowers(stimDefs.at(6));
    mStimConfig.waveFile = stimDefs.at(7);
    mStimConfig.repeatCount = std::stoll(stimDefs.at(8));
  }
  ~StimDefInner() { Cleanup(); }

  void SetFreqListFile(const std::string &file) {
    if (file.empty()) {
      throw std::runtime_error("Invalid FreqList file.");
    }
    mFreqList->SetFreqListFile(file);
  }
  void SetFreqListIndex(size_t index) { mFreqList->UpdateFreqListIndex(index); }
  void SetFreqListIndex(const std::vector<size_t> &indexs) { mFreqList->UpdateFreqListIndex(indexs); }
  void UpdateFreqListValuesByName(const std::string &name) {
    mFreqList->SetFreqListName(name);
    mFreqList->UpdateFreqListValues();
  }
  std::string FreqListFile() const { return mFreqList->GetFreqListFile(); }
  std::string FreqListName() const { return mStimConfig.freqListName; }
  std::string StimName() const { return mStimConfig.stimName; }
  std::string Pin() const { return mStimConfig.pinName; }
  std::string Type() const { return mStimConfig.stimType; }
  std::string WaveFile() const { return mStimConfig.waveFile; }
  std::vector<double> Power() const { return mStimConfig.powers; }
  std::vector<size_t> FreqListIndex() const { return mStimConfig.freqListIndexs; }
  double Frequency() const { return mFreqList->GetFreqByIndex(mStimConfig.freqListIndexs[0]); }
  std::vector<double> Frequencies() const { return mFreqList->GetFreqsByIndexs(mStimConfig.freqListIndexs); }
  double GetFrequencyByIndex(size_t index) { return mFreqList->GetFreqByIndex(index); }
  std::vector<double> GetFrequencyListByIndex(const std::vector<size_t> &indexs) {
    return mFreqList->GetFreqsByIndexs(indexs);
  }

  std::vector<std::string> GetStimDefs() const { return mStimDefs; }

protected:
  void Cleanup() { mFreqList->Cleanup(); }
  std::vector<double> ParsePowers(const std::string &power) {
    auto power_tokens = RFUTILS::DoSplit(power, '|');
    std::vector<double> powers(power_tokens.size());
    std::transform(power_tokens.begin(), power_tokens.end(), std::back_inserter(powers),
                   [](auto &token) { return std::stod(token); });
    return powers;
  }

private:
  std::vector<std::string> mStimDefs;
  StimConfiguration mStimConfig;
  std::shared_ptr<FreqListInner> mFreqList = nullptr;
};

#endif