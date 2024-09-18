#ifndef FREQ_LIST_INNER_H
#define FREQ_LIST_INNER_H

#include <algorithm>
#include <string>
#include <vector>

#include "kits/utils.hpp"

class FreqListInner {
public:
  explicit FreqListInner(const std::string &flist, const std::string &flistIndex)
      : mFreqListName(flist), mFreqListIndex(flistIndex) {}
  ~FreqListInner() = default;

  void SetFreqListFile(const std::string &file) { mFreqListFile = file; }
  void SetFreqListName(const std::string &flistName) { mFreqListName = flistName; }
  void UpdateFreqListIndex(const std::string &index) { mFreqListIndex = index; }
  void UpdateFreqListIndex(size_t index) { mFreqListIndex = std::to_string(index); }
  void UpdateFreqListIndex(const std::vector<size_t> &indexs) {
    std::vector<std::string> indexStrs(indexs.size());
    std::transform(indexs.begin(), indexs.end(), std::back_inserter(indexStrs),
                   [](size_t index) { return std::to_string(index); });
    mFreqListIndex = RFUTILS::DoJoin(indexStrs, "|");
  }
  void UpdateFreqListValues() {
    auto queryResult = RFUTILS::DoQuery(mFreqListFile, mFreqListName);
    if (queryResult.empty()) {
      throw std::runtime_error("Not Found Such FreqListName: " + mFreqListName);
    }
    auto flistValues = RFUTILS::DoSplit(queryResult[1], '|');
    mFreqListValues = ConvertToNumeric<double>(flistValues);
  }

  std::string GetFreqListFile() const { return mFreqListFile; }
  std::string GetFreqListName() const { return mFreqListName; }
  std::string GetFreqListIndexStr() const { return mFreqListIndex; }
  double GetFreqByIndex(size_t index) {
    if (index >= mFreqListValues.size()) {
      throw std::runtime_error("Index out of range.");
    }
    return mFreqListValues[index];
  }
  std::vector<double> GetFreqsByIndexs(const std::vector<size_t> &indexs) {
    std::vector<double> newFreqListValue{};
    for (const auto &index : indexs) {
      auto value = GetFreqByIndex(index);
      newFreqListValue.push_back(value);
    }
    return newFreqListValue;
  }
  std::vector<size_t> GetFreqListIndexs() {
    auto flistIndexs = RFUTILS::DoSplit(mFreqListIndex, '|');
    return ConvertToNumeric<size_t>(flistIndexs);
  }
  size_t GetFreqListIndex() {
    size_t index = std::stoul(mFreqListIndex);
    if (index >= mFreqListValues.size()) {
      throw std::runtime_error("Index out of range!");
    }
    return index;
  }
  void Cleanup() {
    std::string().swap(mFreqListName);
    std::string().swap(mFreqListFile);
    std::string().swap(mFreqListIndex);
    std::vector<double>().swap(mFreqListValues);
  }

protected:
  template <typename T> std::vector<T> ConvertToNumeric(const std::vector<std::string> &freqs) {
    static_assert(std::is_arithmetic_v<T>, "Template parameter T must be a numeric type.");
    std::vector<T> numericValues(freqs.size());
    for (const auto &freq : freqs) {
      try {
        if constexpr (std::is_same_v<T, double>) {
          numericValues.emplace_back(std::stod(freq));
        } else if constexpr (std::is_same_v<T, float>) {
          numericValues.emplace_back(std::stof(freq));
        } else if constexpr (std::is_integral_v<T>) {
          numericValues.emplace_back(static_cast<T>(std::stoull(freq)));
        } else {
          throw std::runtime_error("Unsupported numeric type.");
        }
      } catch (const std::invalid_argument &e) {
        throw std::runtime_error("Conversion value failed for: " + freq + ", error: " + std::string(e.what()));
      } catch (const std::out_of_range &e) {
        throw std::runtime_error("Conversion value out of range for: " + freq + ", error: " + std::string(e.what()));
      }
    }
    return numericValues;
  }

private:
  std::string mFreqListName;
  std::string mFreqListIndex;
  std::string mFreqListFile;
  std::vector<double> mFreqListValues;
};

#endif // FREQ_LIST_INNER_H