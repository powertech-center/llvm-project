#ifndef LLDB_TOOLS_LLDB_VSCODE_PIPEBASESC_H
#define LLDB_TOOLS_LLDB_VSCODE_PIPEBASESC_H 

#include "llvm/ADT/StringRef.h"
#include "lldb/Utility/Status.h"

namespace lldb_vscode {

class PipeBaseSC {
public:
    virtual lldb_private::Status Create(llvm::StringRef name) = 0;
    virtual lldb_private::Status OpenExist(llvm::StringRef name) = 0;
    virtual lldb_private::Status Connect() = 0;
    virtual lldb_private::Status Disconnect() = 0;
    
    virtual void Close() = 0;

    virtual lldb_private::Status Write(void const* buff, size_t bytes_count, size_t& bytes_writed) = 0;
    virtual lldb_private::Status Read(void* buff, size_t buff_size, size_t& bytes_readed) const = 0;

    virtual llvm::StringRef GetPath() const = 0;
};

} // namespace lldb_vscode

#endif