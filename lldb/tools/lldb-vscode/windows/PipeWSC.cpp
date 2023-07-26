#include "PipeWSC.h"

using namespace lldb_private;
using namespace llvm;
using namespace lldb;
using namespace lldb_vscode;

static const std::string PIPE_PATH{"\\\\.\\Pipe\\"};

PipeWSC::PipeWSC() : m_pipe{INVALID_HANDLE_VALUE} {}
PipeWSC::~PipeWSC() { Close(); }

Status PipeWSC::Create(StringRef name) {
  if (m_pipe != INVALID_HANDLE_VALUE) {
    Close();
  }

  auto path{PIPE_PATH};
  path.append(name.data());

  SECURITY_ATTRIBUTES attrs{sizeof(SECURITY_ATTRIBUTES), nullptr, false};

  m_pipe = CreateNamedPipeA(path.data(), PIPE_ACCESS_DUPLEX,
                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                            PIPE_UNLIMITED_INSTANCES, 1024, 1024, 120 * 1000,
                            &attrs);

  if (m_pipe == INVALID_HANDLE_VALUE) {
    return Status(GetLastError(), eErrorTypeWin32);
  }

  m_path = std::move(path);

  return Status();
}

Status PipeWSC::OpenExist(StringRef name) {
  if (m_pipe != INVALID_HANDLE_VALUE) {
    Close();
  }

  auto path{PIPE_PATH};
  path.append(name.data());

  m_pipe = CreateFileA(name.data(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                       OPEN_EXISTING, 0, nullptr);

  if (m_pipe == INVALID_HANDLE_VALUE) {
    return Status(GetLastError(), eErrorTypeWin32);
  }

  m_path = std::move(path);

  return Status();
}

Status PipeWSC::Connect() {
  if (!ConnectNamedPipe(m_pipe, nullptr)) {
    return Status(GetLastError(), eErrorTypeWin32);
  }
  
  return Status();
}

Status PipeWSC::Disconnect() {
  if (!DisconnectNamedPipe(m_pipe)) {
    return Status(GetLastError(), eErrorTypeWin32);
  }

  return Status();
}

void PipeWSC::Close() {
  FlushFileBuffers(m_pipe);
  DisconnectNamedPipe(m_pipe);
  CloseHandle(m_pipe);
  m_pipe = INVALID_HANDLE_VALUE;
}

Status PipeWSC::Write(void const* buff, size_t bytes_count, size_t &bytes_writed) {
  if (!WriteFile(m_pipe, buff, bytes_count, (LPDWORD) &bytes_writed, nullptr)) {
    return Status(GetLastError(), eErrorTypeWin32);
  }

  return Status();
}

Status PipeWSC::Read(void* buff, size_t buff_size, size_t& bytes_readed) const {
  if (!ReadFile(m_pipe, buff, buff_size, (LPDWORD) &bytes_readed, nullptr)) {
    return Status(GetLastError(), eErrorTypeWin32);
  }

  return Status();
}

StringRef PipeWSC::GetPath() const { return m_path; }
