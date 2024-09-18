#include "kits/csvparser.hpp"
#include <algorithm>

namespace CSVUtils {
inline namespace FileOperations {
bool CheckFileExtension(const std::string &filename, const std::string &ext) {
  // return filename.substr(filename.find_last_of(".")) == ext;
  size_t dotPos = filename.find_last_of(".");
  if (dotPos == std::string::npos || dotPos == 0)
    return false;
  std::string file_ext = filename.substr(dotPos);
  if (file_ext.size() != ext.size())
    return false;
  return std::equal(
      file_ext.begin(), file_ext.end(), ext.begin(),
      [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}
size_t CalFileByteSize(std::ifstream &in) {
  in.seekg(0, std::ios::end);
  size_t file_byte_size = in.tellg();
  in.seekg(0, std::ios::beg);
  return file_byte_size;
}
std::unique_ptr<BaseIO> CreateFileHandler(std::ifstream &in) {
  if (!in.good())
    throw std::runtime_error("Invalid file handle.");
  return std::make_unique<IStreamIO>(in);
}
}; // namespace FileOperations

inline namespace ParseOperations {
std::vector<std::string_view> SplitRow(std::string_view row, const char &ch) {
  std::vector<std::string_view> tmp;
  tmp.reserve(row.size() / 2);
  size_t start = 0;

  bool IsNewLine = false;
  bool IsStartPosition = false;
  bool IsEndPosition = false;

  for (size_t pos = 0; pos < row.size(); pos++) {
#ifdef _WIN32
    IsNewLine =
        (row[pos] == '\r' && pos + 1 < str.size() && row[pos + 1] == '\n');
    IsStartPosition = (row[pos] == start && row[pos] == ch);
#else
    IsNewLine = (row[pos] == '\n');
    IsStartPosition = (pos == start && row[pos] == ch && !IsNewLine);
#endif
    if (row[pos] == ch || IsNewLine) {
      if (pos > start || IsStartPosition) {
        tmp.emplace_back(row.substr(start, pos - start));
      }
      start = pos + (row[pos] == '\r' ? 2 : 1);
      if (IsNewLine) {
        ++pos;
      }
    }
  }
  IsEndPosition = (start == row.size() && row[start - 1] == ch && ch != '\n');
  // process the last character: empty or not
  if (start < row.size() || IsEndPosition) {
    tmp.emplace_back(row.substr(start));
  }
  return tmp;
}
std::vector<std::string_view> SplitSkipHeaderRow(std::string_view row,
                                                 const char &ch) {
  auto pos = row.find_first_of(ch);
  return SplitRow(row.substr(pos + 1), ch);
}
std::string_view SplitHeaderRow(std::string_view header, const char &ch) {
  auto pos = header.find_first_of(ch);
  return header.substr(0, pos);
}
}; // namespace ParseOperations
}; // namespace CSVUtils

FileHandle::FileHandle(const std::string &filename)
    : m_file(filename), m_handle(m_file, std::ios::binary) {
  if (!m_handle.is_open()) {
    throw std::runtime_error("Failed to open file.");
  }
}

FileHandle::~FileHandle() {
  if (m_handle.is_open()) {
    m_handle.close();
  }
}

FileManager::FileManager(const std::string &filename)
    : m_file_handle(std::make_unique<FileHandle>(filename)) {}

void FileManager::OpenSourceFile(const std::string &filename) {
  if (!m_file_handle)
    throw std::runtime_error("File handle is not initialized.");
  m_file_size =
      CSVUtils::FileOperations::CalFileByteSize(m_file_handle->GetHandle());
}

std::unique_ptr<BaseIO> FileManager::CreateFileHandler() {
  return CSVUtils::FileOperations::CreateFileHandler(
      m_file_handle->GetHandle());
}

ParserImpl::ParserImpl() { Initialize(); }

void ParserImpl::SetColumnNames(const std::vector<std::string_view> &colNames) {
  m_col_names = colNames;
}

void ParserImpl::ParseRows(const std::unique_ptr<BaseIO> &io, size_t filesize) {
  m_read_buffer.resize(filesize);
  io->Read(m_read_buffer.data(), filesize);
  if (m_read_buffer.size() == 0)
    throw std::runtime_error("No context.");
  m_rows = CSVUtils::ParseOperations::SplitSkipHeaderRow(m_read_buffer, '\n');
}

void ParserImpl::ParseColumns(const std::vector<std::string_view> &rows) {
  m_data.reserve(rows.size());
  for (size_t i = 0; i < rows.size(); ++i) {
    auto columns = CSVUtils::ParseOperations::SplitRow(rows[i], ',');
    if (!m_col_names.empty() && !ValidateColumnSize(columns)) {
      auto err = std::string("Invalid column size: ") +
                 std::to_string(columns.size()) + " at row " +
                 std::to_string(i);
      throw std::runtime_error(err);
    }
    m_data[i] = columns;
  }
}

void ParserImpl::AsyncParseColumns(const std::vector<std::string_view> &rows,
                                   size_t workers) {
  std::vector<std::thread> worker_threads;
  std::atomic<size_t> counter{0};
  std::atomic<bool> hasException{false};
  std::queue<std::exception_ptr> exceptions;
  m_data.resize(rows.size());

  auto CheckAndSplitRow2Columns = [this, &rows](size_t j) {
    auto columns = CSVUtils::ParseOperations::SplitRow(rows[j], ',');
    if (!this->m_col_names.empty() && !ValidateColumnSize(columns)) {
      auto err = std::string("Invalid column size: ") +
                 std::to_string(columns.size()) + " at row " +
                 std::to_string(j);
      throw std::runtime_error(err);
    }
    return std::vector{columns};
  };

  for (size_t i = 0; i < workers; ++i) {
    worker_threads.emplace_back([&counter, &hasException, &exceptions,
                                 &CheckAndSplitRow2Columns, rows, this] {
      while (true) {
        size_t j = counter.fetch_add(1);
        if (j >= rows.size() || hasException.load())
          break;
        try {
          m_data[j] = CheckAndSplitRow2Columns(j);
        } catch (...) {
          exceptions.push(std::current_exception());
          hasException.store(true);
        }
      }
    });
  }

  for (auto &worker : worker_threads) {
    worker.join();
  }
  if (hasException.load() && !exceptions.empty()) {
    std::rethrow_exception(exceptions.front());
  }
}

void ParserImpl::Initialize() {
  m_read_buffer = "";
  m_rows.clear();
  m_data.clear();
  m_col_names.clear();
}

void ParserImpl::ClearAllCache() {
  std::string().swap(m_read_buffer);
  std::vector<std::string_view>().swap(m_rows);
  std::vector<std::string_view>().swap(m_col_names);
  std::vector<std::vector<std::string_view>>().swap(m_data);
}

void ParserImpl::WriteToFile(const std::string &filename) {
  std::ofstream out(filename, std::ios::out);
  if (!out.is_open()) {
    throw std::runtime_error("Failed to open file.");
  }

  for (const auto &col : m_col_names) {
    out << col;
    if (&col != &m_col_names.back()) {
      out << ",";
    }
  }
  out << std::endl;

  for (const auto &row : m_data) {
    for (const auto &col : row) {
      out << col;
      if (&col != &row.back()) {
        out << ",";
      }
    }
    out << std::endl;
  }
  out.close();
}

void ParserImpl::OnOperationCallback(OperateStrategyCallback on_operation) {
  if (on_operation)
    on_operation(m_data);
}

std::any ParserImpl::OnQueryCallback(QueryStrategyCallback on_query) {
  if (on_query)
    return on_query(m_data);
  return std::any{};
}

bool ParserImpl::ValidateColumnSize(
    const std::vector<std::string_view> &columns) {
  return columns.size() == m_col_names.size();
}

ParserStrategy::ParserStrategy()
    : m_parser_impl(std::make_unique<ParserImpl>()) {}

void ParserStrategy::SetColumnNames(
    const std::vector<std::string_view> &columns) {
  m_parser_impl->SetColumnNames(columns);
}
void ParserStrategy::WriteToFile(const std::string &filename) {
  m_parser_impl->WriteToFile(filename);
}

void ParserStrategy::ClearAllCache() { m_parser_impl->ClearAllCache(); }

void ParserStrategy::OnOperationCallback(OperateStrategyCallback on_operation) {
  m_parser_impl->OnOperationCallback(on_operation);
}

std::any ParserStrategy::OnQueryCallback(QueryStrategyCallback on_query) {
  return m_parser_impl->OnQueryCallback(on_query);
}

void SynchronousParser::ParseDataFromCSV(const std::unique_ptr<BaseIO> &io,
                                         size_t filesize) {
  m_parser_impl->ParseRows(std::move(io), filesize);
  m_parser_impl->ParseColumns(m_parser_impl->GetAllRows());
}

AsynchronousParser::AsynchronousParser(size_t workers)
    : m_thread_workers(workers) {}

void AsynchronousParser::ParseDataFromCSV(const std::unique_ptr<BaseIO> &io,
                                          size_t filesize) {
  m_parser_impl->ParseRows(std::move(io), filesize);
  m_parser_impl->AsyncParseColumns(m_parser_impl->GetAllRows(),
                                   m_thread_workers);
}

void CSVParser::SetParser(ParserMode mode, size_t workers) {
  switch (mode) {
  case ParserMode::Asynchronous:
    m_parser = std::make_unique<AsynchronousParser>(workers);
    break;
  case ParserMode::Synchronous:
    m_parser = std::make_unique<SynchronousParser>();
    break;
  default:
    throw std::runtime_error("Invalid parser mode.");
  }
}

void CSVParser::ParseFromCSV(const std::string &filename) {
  auto fileManager = std::make_unique<FileManager>(filename);
  auto fileHandler = fileManager->CreateFileHandler();
  m_parser->ParseDataFromCSV(fileHandler, fileManager->GetFileSize());
}

std::vector<std::string_view> CSVParser::GetColumnNames() const noexcept {
  return m_parser->GetColumnNames();
}

std::vector<std::vector<std::string_view>>
CSVParser::GetCSVData() const noexcept {
  return m_parser->GetCSVData();
}

size_t CSVParser::GetDataSize() const noexcept {
  return m_parser->GetCSVDataSize();
}

void CSVParser::WriteToCSV(const std::string &filename) {
  m_parser->WriteToFile(filename);
}

void CSVParser::Close() { m_parser->ClearAllCache(); }

void CSVParser::OnAdd(OperateStrategyCallback add) {
  m_parser->OnOperationCallback(add);
}

void CSVParser::OnDelete(OperateStrategyCallback del) {
  m_parser->OnOperationCallback(del);
}

void CSVParser::OnUpdate(OperateStrategyCallback update) {
  m_parser->OnOperationCallback(update);
}

std::any CSVParser::OnQuery(QueryStrategyCallback query) {
  return m_parser->OnQueryCallback(query);
}
