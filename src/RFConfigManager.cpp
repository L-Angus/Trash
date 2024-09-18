#include "impl/RFConfigManager.h"
#include "kits/csvparser.hpp"

ConfiguratorListener::ConfiguratorListener(const std::string &filename)
    : m_config(filename), m_parser_proxy(std::make_shared<CSVParser>(ParserMode::Synchronous)) {}

ConfiguratorListener::~ConfiguratorListener() { this->Cleanup(); }

void ConfiguratorListener::HandleEvent(EventType type, QuerySequence &query) {
  if (type == EventType::CONFIG_LOAD) {
    OnLoadEvent(m_config);
  } else {
    if (!IsConfigLoaded()) {
      throw std::runtime_error("ConfiguratorListener::HandleEvent: Configuration file not loaded yet");
    }
    if (type == EventType::CONFIG_QUERY) {
      OnQueryEvent(query);
    }
  }
}

void ConfiguratorListener::OnLoadEvent(const std::string &filename) {
  m_parser_proxy->ParseFromCSV(filename);
  SetConfigLoadStatus(true);
}

void ConfiguratorListener::OnQueryEvent(QuerySequence &query) {
  auto queryStrategy = [&query](const std::vector<std::vector<std::string_view>> &data) {
    for (const auto &row : data) {
      if (std::string{row[0]} == query.queryCommand) {
        return row;
      }
    }
    return std::vector<std::string_view>{};
  };
  auto result = m_parser_proxy->OnQuery(queryStrategy);
  if (!result.has_value()) {
    throw std::runtime_error("ConfiguratorListener::OnQueryEvent: No result found for query");
  }
  try {
    query.queryResult = std::any_cast<std::vector<std::string_view>>(result);
  } catch (const std::bad_any_cast &e) {
    throw std::runtime_error("ConfiguratorListener::OnQueryEvent: Invalid query result type");
  }
}

void ConfiguratorListener::SetConfigLoadStatus(bool status) { isConfigLoaded = status; }

void ConfiguratorListener::Cleanup() {
  m_config = "";
  SetConfigLoadStatus(false);
  m_parser_proxy->Close();
}

StimConfigurator::StimConfigurator(const std::string &stim) : ConfiguratorListener(stim) {
  m_parser_proxy->SetColumnNames("StimName", "StimType", "TriggerType", "PinName", "FreqListName", "FreqListIndex",
                                 "Power", "WaveFile");
}

MeasConfigurator::MeasConfigurator(const std::string &meas) : ConfiguratorListener(meas) {
  m_parser_proxy->SetColumnNames("MeasName", "TriggerType", "PinName", "FreqListName", "FreqListIndex", "Power");
}

FlistConfigurator::FlistConfigurator(const std::string &flist) : ConfiguratorListener(flist) {
  m_parser_proxy->SetColumnNames("FreqListName", "FreqListValue");
}

void EventPublisher::AddListener(const std::string &config, const EventListenerPtr &listener) {
  mActiveListeners.try_emplace(config, listener);
}

void EventPublisher::RemoveOne(const std::string &config) { mActiveListeners.erase(config); }

void EventPublisher::RemoveAll() { mActiveListeners.clear(); }

void EventPublisher::NotifyAll(EventType type, QuerySequence &maybeUsed) {
  for (const auto &[config, listener] : mActiveListeners) {
    listener->HandleEvent(type, maybeUsed);
  }
}

void EventPublisher::NotifyOne(const std::string &config, EventType type, QuerySequence &maybeUsed) {
  auto listener = mActiveListeners.find(config);
  if (listener == mActiveListeners.end()) {
    throw std::runtime_error("EventPublisher::NotifyOne: Listener not found");
  }
  listener->second->HandleEvent(type, maybeUsed);
}

RFConfigManager::RFConfigManager() {}
RFConfigManager::~RFConfigManager() {}

RFConfigManager &RFConfigManager::GetInstance() {
  static RFConfigManager instance;
  return instance;
}

void RFConfigManager::SetupConfiguratorFactory() {
  mConfiguratorFactory[".stim"] = [](const std::string &stim) { return std::make_unique<StimConfigurator>(stim); };
  mConfiguratorFactory[".meas"] = [](const std::string &meas) { return std::make_unique<MeasConfigurator>(meas); };
  mConfiguratorFactory[".flist"] = [](const std::string &flist) { return std::make_unique<FlistConfigurator>(flist); };
}
void RFConfigManager::DestoryConfiguratorFactory() {
  mPublisher.RemoveAll();
  mConfiguratorFactory.clear();
}

void RFConfigManager::CreateConfigurator(const std::string &config) {
  auto configurator = CreateConfiguratorImpl(config);
  if (!configurator) {
    throw std::runtime_error("RFConfigManager::CreateConfigurator: Failed to create configurator");
  }
  mPublisher.AddListener(config, configurator);
}

void RFConfigManager::CreateConfigurators(const std::vector<std::string> &configs) {
  for (const auto &config : configs) {
    CreateConfigurator(config);
  }
}

void RFConfigManager::OnLoad(const std::string &config) {
  QuerySequence sequence{"Unused, Just a placeholder", std::vector<std::string_view>{}};
  mPublisher.NotifyOne(config, EventType::CONFIG_LOAD, sequence);
}

void RFConfigManager::OnLoad() {
  QuerySequence sequence{"Unused, Just a placeholder", std::vector<std::string_view>{}};
  mPublisher.NotifyAll(EventType::CONFIG_LOAD, sequence);
}

void RFConfigManager::OnQuery(const std::string &config, QuerySequence &maybeUsed) {
  mPublisher.NotifyOne(config, EventType::CONFIG_QUERY, maybeUsed);
}

std::string RFConfigManager::GetConfigFileByExtension(const std::string &ext) {
  auto listener = mPublisher.GeActivetListeners();
  for (const auto &[config, listener] : listener) {
    size_t pos = config.find_last_of(".");
    if (pos == std::string::npos) {
      throw std::runtime_error("RFConfigManager::GetConfigFileByExtension: Invalid file extension");
    }
    if (config.substr(pos) == ext) {
      return config;
    }
  }
  throw std::runtime_error("RFConfigManager::GetConfigFileByExtension: No config file found");
}

EventListenerPtr RFConfigManager::CreateConfiguratorImpl(const std::string &config) {
  size_t pos = config.find_last_of(".");
  if (pos == std::string::npos) {
    throw std::runtime_error("RFConfigManager::CreateConfiguratorImpl: Invalid file extension");
  }
  std::string suffix = config.substr(pos);
  auto it = mConfiguratorFactory.find(suffix);
  if (it == mConfiguratorFactory.end()) {
    throw std::runtime_error("RFConfigManager::CreateConfiguratorImpl: Unsupported file extension");
  }
  return it->second(config);
}