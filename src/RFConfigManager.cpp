#include "impl/RFConfigManager.h"
#include "kits/csvparser.hpp"

ConfiguratorListener::ConfiguratorListener(const std::string& filename) : m_config(filename), 
m_parser_proxy(std::make_shared<CSVParser>(ParserMode::Synchronous)){}

ConfiguratorListener::~ConfiguratorListener(){ this->Cleanup(); }

void ConfiguratorListener::HandleEvent(EventType type, QuerySequence& query){
    if(type == EventType::CONFIG_LOAD){
        OnLoadEvent(m_config);
    }else{
        if(!IsConfigLoaded()){
            throw std::runtime_error("ConfiguratorListener::HandleEvent: Configuration file not loaded yet");
        }
        if(type == EventType::CONFIG_QUERY){
            OnQueryEvent(query);
        }
    }
}

void ConfiguratorListener::OnLoadEvent(const std::string& filename){
    m_parser_proxy->ParseFromCSV(filename);
    SetConfigLoadStatus(true);
}

void ConfiguratorListener::OnQueryEvent(QuerySequence& query){
    auto queryStrategy = [&query](const std::vector<std::vector<std::string_view>>& data){
        for(const auto& row : data){
            if(std::string{row[0]} ==  query.queryCommand){
                return row;
            }
        }
        return std::vector<std::string_view>{};
    };
    auto result = m_parser_proxy->OnQuery(queryStrategy);
    if(!result.has_value()){
        throw std::runtime_error("ConfiguratorListener::OnQueryEvent: No result found for query");
    }
    try{
        query.queryResult = std::any_cast<std::vector<std::string_view>>(result);
    }catch(const std::bad_any_cast& e){
        throw std::runtime_error("ConfiguratorListener::OnQueryEvent: Invalid query result type");
    }
}

void ConfiguratorListener::SetConfigLoadStatus(bool status){
    isConfigLoaded = status;
}

void ConfiguratorListener::Cleanup(){
    m_config = "";
    SetConfigLoadStatus(false);
    m_parser_proxy->Close();
}