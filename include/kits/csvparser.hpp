#ifndef CSV_PARSER_HPP
#define CSV_PARSER_HPP

#include <fstream>
#include <memory>
#include <vector>
#include <functional>
#include <any>
#include <set>
#include <thread>
#include <queue>

class BaseIO{
    public:
     virtual size_t Read(char* buffer, size_t filesize) = 0;
     virtual ~BaseIO() = default;
};

class IStreamIO : public BaseIO{
    public:
    IStreamIO(std::istream& stream) : m_stream(stream){}
    size_t Read(char* buffer, size_t filesize) override{
        m_stream.read(buffer, filesize);
        return m_stream.gcount();
    }
    private:
    std::istream& m_stream;
};

class CFileIO : public BaseIO{
    public:
    // TODO: Add support for C files handler 
};

namespace CSVUtils{
    inline namespace FileOperations{
        bool CheckFileExtension(const std::string& filename, const std::string& ext);
        size_t CalFileByteSize(std::ifstream& in);
        std::unique_ptr<BaseIO> CreateFileHandler(std::ifstream& in);
    };

    inline namespace ParseOperations{
        std::vector<std::string_view> SplitRow(std::string_view row, const char& ch);
        std::vector<std::string_view> SplitSkipHeaderRow(std::string_view row, const char& ch);
        std::string_view SplitHeaderRow(std::string_view header, const char& ch);
    };
};

class FileHandle{
    public:
    explicit FileHandle(const std::string& filename);
    ~FileHandle();

    std::ifstream& GetHandle() { return m_handle; }
    const std::string& GetHandleContext() const { return m_file; }

    private:
    std::string m_file;
    std::ifstream m_handle;
};

class FileManager{
    public:
    explicit FileManager(const std::string& filename);
    ~FileManager() =  default;

    std::unique_ptr<BaseIO> CreateFileHandler();
    size_t GetFileSize() const noexcept { return m_file_size; }
    std::string GetFileName() const noexcept { return m_file_handle->GetHandleContext(); }

    private:
    void OpenSourceFile(const std::string& filename);
    size_t m_file_size = 0;
    std::unique_ptr<FileHandle> m_file_handle = nullptr;
};

using OperateStrategyCallback = std::function<void(std::vector<std::vector<std::string_view>>&)>;
using QueryStrategyCallback = std::function<std::any(const std::vector<std::vector<std::string_view>>&)>;

class ParserImpl{
    public:
    ParserImpl();
    void SetColumnNames(const std::vector<std::string_view>& colNames); // set the first column
    void ParseRows(const std::unique_ptr<BaseIO>& io, size_t filesize);
    void ParseColumns(const std::vector<std::string_view>& rows);
    void AsyncParseColumns(const std::vector<std::string_view>& rows, size_t workers);
    void WriteToFile(const std::string& filename);
    void ClearAllCache();

    std::vector<std::string_view> GetColumnNames() const noexcept { return m_col_names; }
    std::vector<std::string_view> GetAllRows() const noexcept { return m_rows; }
    std::vector<std::vector<std::string_view>> GetCSVData() const noexcept { return m_data; }
    size_t GetRowsSize() const noexcept { return m_rows.size(); }
    size_t GetCSVDataSize() const noexcept { return m_data.size(); }

    void OnOperationCallback(OperateStrategyCallback on_operation);
    std::any OnQueryCallback(QueryStrategyCallback on_query);

    private:
    void Initialize();
    bool ValidateColumnSize(const std::vector<std::string_view>& columns);

    std::string m_read_buffer;
    std::vector<std::string_view> m_rows;
    std::vector<std::string_view> m_col_names;
    std::vector<std::vector<std::string_view>> m_data;
};

class ParserStrategy{
    public:
    ParserStrategy();
    virtual ~ParserStrategy() = default;

    /* Synchronous && Asynchronous public operations */
    void SetColumnNames(const std::vector<std::string_view>& colNames);
    std::vector<std::string_view> GetColumnNames() const noexcept { return m_parser_impl->GetColumnNames(); }
    std::vector<std::vector<std::string_view>> GetCSVData() const noexcept{ return m_parser_impl->GetCSVData(); }
    void WriteToFile(const std::string& filename);
    size_t GetRowsSize() const noexcept { return m_parser_impl->GetRowsSize(); }
    size_t GetCSVDataSize() const noexcept { return m_parser_impl->GetCSVDataSize(); }
    void ClearAllCache();

    void OnOperationCallback(OperateStrategyCallback on_operation);
    std::any OnQueryCallback(QueryStrategyCallback on_query);

    /* Synchronous && Asynchronous unique operations */
    virtual void ParseDataFromCSV(const std::unique_ptr<BaseIO>& io, size_t filesize) = 0;

    protected:
    std::unique_ptr<ParserImpl> m_parser_impl = nullptr;
};

class SynchronousParser : public ParserStrategy{
    public:
    void ParseDataFromCSV(const std::unique_ptr<BaseIO>& io, size_t filesize) override;
};

class AsynchronousParser : public ParserStrategy{
    public:
    AsynchronousParser(size_t workers = std::thread::hardware_concurrency());
    void ParseDataFromCSV(const std::unique_ptr<BaseIO>& io, size_t filesize) override;

    private:
    size_t m_thread_workers = 0;
};

enum class ParserMode { Synchronous, Asynchronous };

class CSVParser{
    using CSVData = std::vector<std::vector<std::string_view>>;
    public:
    CSVParser() = default;
    CSVParser(const CSVParser& ) = delete;
    CSVParser& operator=(const CSVParser& ) = delete;
    ~CSVParser() = default;

    template<typename... Args>
    explicit CSVParser(ParserMode parser, Args&&... args){
        SetParser(parser);
        if constexpr(sizeof...(Args) > 0){
            SetColumnNames(std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    void SetColumnNames(Args&&... args){
        std::initializer_list<std::string_view> columnNames{std::forward<Args>(args)...};
        if(columnNames.size() == 0) throw std::runtime_error("No column names provided.");
        std::set<std::string_view> unique_columns(columnNames.begin(), columnNames.end());
        if(unique_columns.size() != columnNames.size()) throw std::runtime_error("Duplicate column names provided.");
        m_parser->SetColumnNames(columnNames);
    }

    void ParseFromCSV(const std::string& filename);
    void WriteToCSV(const std::string& filename);
    void SetParser(ParserMode mode, size_t workers = std::thread::hardware_concurrency());
    void OnAdd(OperateStrategyCallback add);
    void OnDelete(OperateStrategyCallback del);
    void OnUpdate(OperateStrategyCallback update);
    std::any OnQuery(QueryStrategyCallback query);

    std::vector<std::string_view> GetColumnNames() const noexcept;
    std::vector<std::vector<std::string_view>> GetCSVData() const noexcept;
    size_t GetDataSize() const noexcept;
    void Close();

    private:
    std::unique_ptr<ParserStrategy> m_parser = nullptr;
    
};

#endif