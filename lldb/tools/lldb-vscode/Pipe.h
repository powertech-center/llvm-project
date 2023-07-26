#ifndef LLDB_TOOLS_LLDB_VSCODE_PIPE_H
#define LLDB_TOOLS_LLDB_VSCODE_PIPE_H

#include <memory>

#if defined(_WIN32)

#include "windows/PipeWSC.h"

namespace lldb_vscode {
using Pipe = PipeWSC;
} // namespace lldb_vscode

#else

#include "posix/PipePSC.h"

namespace lldb_vscode {
using Pipe = PipeWSC;   
} // namespace lldb_vscode

#endif

namespace lldb_vscode {
using PipeUP = std::unique_ptr<Pipe>;
} // namespace lldb_vscode
#endif