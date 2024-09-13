#ifndef RF_CONFIG_MANAGER_H
#define RF_CONFIG_MANAGER_H

#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <functional>

enum class EventType{ CONFIG_LOAD, CONFIG_QUERY };

struct QuerySequence{
    std::string queryCommand;
    std::vector<std::string_view> queryResult;
};

class EventListener;
class CSVParser;

using EventListenerPtr = std::shared_ptr<EventListener>;
using EventListenerWrpper = std::function<EventListenerPtr(const std::string&)>;

class EventListener{
    public:
    virtual void HandleEvent(EventType type, QuerySequence& query) = 0;
    virtual ~EventListener() = default;
};

class ConfiguratorListener : public EventListener{
    public:
    explicit ConfiguratorListener(const std::string& configFilePath);
    ~ConfiguratorListener();
    virtual void HandleEvent(EventType type, QuerySequence& query) override;

    protected:
    std::string m_config;
    std::shared_ptr<CSVParser> m_parser_proxy = nullptr;
    bool isConfigLoaded{false};

    private:
    void OnLoadEvent(const std::string& filename);
    void OnQueryEvent(QuerySequence& query);
    void SetConfigLoadStatus(bool status);
    bool IsConfigLoaded() const { return isConfigLoaded; }
    void Cleanup();
};

class StimConfigurator : public ConfiguratorListener{
    public:
    explicit StimConfigurator(const std::string& configFilePath);
};
class MeasConfigurator : public ConfiguratorListener{
    public:
    explicit MeasConfigurator(const std::string& configFilePath);
};
class FlistConfigurator : public ConfiguratorListener{
    public:
    explicit FlistConfigurator(const std::string& configFilePath);
};

class EventPublisher{
    public:
    void AddListener(const std::string& config, const EventListenerPtr& listener);
    void RemoveOne(const std::string& config);
    void RemoveAll();
    void NotifyOne(const std::string& config, EventType type, QuerySequence& maybeUsed);
    void NotifyAll(EventType type, QuerySequence& maybeUsed);

    private:
    std::unordered_map<std::string, EventListenerPtr> mActiveListeners;
};

class RFConfigManager{
    public:
    static RFConfigManager& GetInstance();

    void SetupConfiguratorFactory();
    void DestoryConfiguratorFactory();
    void CreateConfigurator(const std::string& config);
    void CreateConfigurators(const std::vector<std::string>& configs);
    void OnLoad(const std::string& config);
    void OnLoad();
    void OnQuery(const std::string& config, QuerySequence& maybeUsed);

    protected:
    EventListenerPtr CreateConfiguratorImpl(const std::string& config);
    EventPublisher mPublisher;
    std::unordered_map<std::string, EventListenerWrpper> mConfiguratorFactory;

    private:
    RFConfigManager();
    ~RFConfigManager();
    RFConfigManager(const RFConfigManager&) = delete;
    RFConfigManager& operator=(const RFConfigManager&) = delete;
};

#endif