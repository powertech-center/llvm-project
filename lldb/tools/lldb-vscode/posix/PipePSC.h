#ifndef LLDB_TOOLS_LLDB_VSCODE_PIPEPSC_H
#define LLDB_TOOLS_LLDB_VSCODE_PIPEPSC_H

#include "../PipeBaseSC.h"

#include <stdio.h>

namespace lldb_vscode {

class PipePSC : public PipeBaseSC {
public:
    PipePSC();
    ~PipePSC();

    lldb_private::Status Create(llvm::StringRef name) override;
    lldb_private::Status OpenExist(llvm::StringRef name) override;
    lldb_private::Status Connect() override;
    lldb_private::Status Disconnect() override;

    void Close() override;

    lldb_private::Status Write(void const* buff, size_t bytes_count, size_t& bytes_writed) override;
    lldb_private::Status Read(void* buff, size_t buff_size, size_t& bytes_readed) const override;

    llvm::StringRef GetPath() const override;

private:
    FILE* m_pipe;
    int m_pipe_fd;
    std::string m_path;
};

} // namespace lldb_vscode

#endif