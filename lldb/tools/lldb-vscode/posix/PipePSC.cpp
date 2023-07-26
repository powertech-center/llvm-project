#include "PipePSC.h"

#include <unistd.h>

using namespace lldb_private;
using namespace lldb;
using namespace lldb_vscode;

PipePSC::PipePSC()
    : m_pipe{nullptr}
    , m_path{} {}

PipePSC::~PipePSC() {
    Close();
}

Status PipePSC::Create(llvm::StringRef name) {
    if (!(m_pipe = mkfifo(name, 0644))) {
        return Status(errno, eErrorTypePOSIX);
    }

    if (!(m_pipe_fd = fileno(m_pipe))) {
        return Status(errno, eErrorTypePOSIX);
    }

    return Status();
}

Status PipePSC::OpenExist(llvm::StringRef name) {
    if (!(m_pipe = fopen(name.data(), "w+"))) {
        return Status(errno, eErrorTypePOSIX);
    }

    if (!(m_pipe_fd = fileno(m_pipe))) {
        return Status(errno, eErrorTypePOSIX);
    }

    return Status();
}

Status PipePSC::Connect() {
    return Status();
}

Status PipePSC::Disconnect() {
    return Status();
}

void PipePSC::Close() {
    if (m_pipe) {
        fclose(m_pipe);
    }
}

Status PipePSC::Write(void const* buff, size_t bytes_count, size_t& bytes_writed) {
    auto succ = write(buff, bytes_count);
    if (succ <= 0 && bytes_count > 0) {
        return Status(errno, eErrorTypePOSIX);
    }
    bytes_writed = succ;
    return Status();
}

Status PipePSC::Read(void* buff, size_t buff_size, size_t& bytes_readed) const {
    auto succ = read(buff, buff_size);
    if (succ <= 0 && buff_size > 0) {
        return Status(errno, eErrorTypePOSIX);
    }
    bytes_readed = succ;
    return Status();
}