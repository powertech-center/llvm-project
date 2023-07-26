#ifndef LLDB_TOOLS_LLDB_VSCODE_PIPEWSC_H
#define LLDB_TOOLS_LLDB_VSCODE_PIPEWSC_H

#include "lldb/Host/windows/windows.h"

#include "../PipeBaseSC.h"

#include <string>

namespace lldb_vscode {

class PipeWSC : public PipeBaseSC {
public:
  PipeWSC();
  ~PipeWSC();

  lldb_private::Status Create(llvm::StringRef name) override;
  lldb_private::Status OpenExist(llvm::StringRef name) override;
  lldb_private::Status Connect() override;
  lldb_private::Status Disconnect() override;

  void Close() override;

  lldb_private::Status Write(void const* buff, size_t bytes_count, size_t& bytes_writed) override;
  lldb_private::Status Read(void* buff, size_t buff_size, size_t& bytes_readed) const override;

  llvm::StringRef GetPath() const override;

private:
  HANDLE m_pipe;
  std::string m_path;
};

} // namesapce lldb_vscode

#endif