#ifndef RF_UTILS_HPP
#define RF_UTILS_HPP

#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "csvparser.hpp"
#include "impl/RFConfigManager.h"

namespace RFUTILS {
std::vector<std::string> DoSplit(const std::string &str, char ch) {
  std::vector<std::string_view> SplitView = CSVUtils::ParseOperations::SplitRow(str, ch);
  return std::vector<std::string>{SplitView.begin(), SplitView.end()};
}
std::string DoJoin(const std::vector<std::string> &strs, const std::string &delim) {
  std::stringstream ss;
  for (size_t i = 0; i < strs.size() - 1; ++i) {
    ss << strs[i] << delim;
  }
  ss << strs[strs.size() - 1];
  return ss.str();
}

std::vector<std::string> DoQuery(const std::string &config, const std::string &queryCommand) {
  std::vector<std::string_view> queryResult;
  QuerySequence sequence{.queryCommand = queryCommand, .queryResult = queryResult};
  RFConfigManager::GetInstance().OnQuery(config, sequence);
  return std::vector<std::string>{sequence.queryResult.begin(), sequence.queryResult.end()};
}
}; // namespace RFUTILS

#endif // RF_UTILS_HPP