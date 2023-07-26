//===-- FifoFiles.cpp -------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FifoFiles.h"

#if !defined(_WIN32)
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <chrono>
#include <fstream>
#include <future>
#include <optional>
#include <thread>
#include <iostream>

#include "llvm/Support/FileSystem.h"

#include "lldb/lldb-defines.h"

using namespace llvm;

namespace lldb_vscode {

#define PIPE_BUFF_SIZE 1024

FifoFile::FifoFile(StringRef other_endpoint_name) 
    : m_pipe{std::make_unique<PipeWSC>()},
      m_other_endpoint_name{other_endpoint_name} {}

lldb_private::Status FifoFile::Create(StringRef path) {
  return m_pipe->Create(path);
}

lldb_private::Status FifoFile::Open(StringRef path) {
  return m_pipe->OpenExist(path);
}

llvm::Expected<std::string> FifoFile::ReadLine() const {
  std::array<char, PIPE_BUFF_SIZE> buff;
  size_t count;
  auto status = m_pipe->Connect();
  status = m_pipe->Read(buff.data(), buff.size(), count);
  if (!status.Success()) {
    return createStringError(
            std::error_code{(int)status.GetError(), std::generic_category()}, 
            "Can't read line in " + m_other_endpoint_name
        );
  }
  m_pipe->Disconnect();
  auto beg = std::cbegin(buff);
  auto end = std::cend(buff);
  auto pos = std::find(beg, end, '\n');

  std::cout << "[INFO -> " __FUNCSIG__ "] Received msg: "
            << std::string{buff.data(), (size_t)std::distance(beg, pos)}
            << std::endl;

  return pos == end ? Expected<std::string>(createStringError(
                          inconvertibleErrorCode(), "Uncorrect line in " + m_other_endpoint_name))
                    : std::string{buff.data(), (size_t) std::distance(beg, pos)};
}

void FifoFile::WriteLine(std::string const& line) {
  std::string buff_l{line};
  buff_l += '\n';

  size_t count = 0;
  auto status = m_pipe->Write(buff_l.data(), buff_l.size(), count);

  if (status.Fail() || count != buff_l.size()) {
    llvm::errs() << createStringError(
        std::error_code{(int)status.GetError(), std::generic_category()},
        "Can't write to io in " + m_other_endpoint_name + " " +
            std::to_string(count) + " " + std::to_string(buff_l.size()));
    exit(EXIT_FAILURE);
  }
  std::cout << "[INFO -> " __FUNCSIG__ "] Sended msg: "
            << buff_l
            << std::endl;
}

std::string FifoFile::GetPath() const { return m_pipe->GetPath().str(); }

FifoFile::~FifoFile() { m_pipe->Close(); }

Expected<FifoFileSP> CreateFifoFile(StringRef path, StringRef other_endpoint_name) {
  auto fifoFile = std::make_shared<FifoFile>(other_endpoint_name);
  auto createResult = fifoFile->Create(path);
  if (createResult.Fail()) {
    createStringError(
        std::error_code((int) createResult.GetError(), std::generic_category()),
        "Couldn't create fifo file: %s", path.data());
  }
  return fifoFile;
}

Expected<FifoFileSP> OpenFifoFile(StringRef path, StringRef other_endpoint_name) {
  auto fifoFile = std::make_shared<FifoFile>(other_endpoint_name);
  auto openResult = fifoFile->Open(path);
  if (openResult.Fail()) {
    return createStringError(
        std::error_code{(int)openResult.GetError(), std::generic_category()},
        "Couldn't open fifo file: %s", path.data());
  }
  return fifoFile;
}

FifoFileIO::FifoFileIO(FifoFileSP io, StringRef other_endpoint_name)
    : m_io(io), m_other_endpoint_name{other_endpoint_name} {}

Expected<json::Value> FifoFileIO::ReadJSON(std::chrono::milliseconds timeout) {
  // We use a pointer for this future, because otherwise its normal destructor
  // would wait for the getline to end, rendering the timeout useless.
  std::optional<std::string> line;
  std::future<void> *future =
      new std::future<void>(std::async(std::launch::async, [&]() {
        if (auto buffer = m_io->ReadLine())
          line = buffer.get();
      }));
  if (future->wait_for(timeout) == std::future_status::timeout || !line)
    // Indeed this is a leak, but it's intentional. "future" obj destructor
    //  will block on waiting for the worker thread to join. And the worker
    //  thread might be stuck in blocking I/O. Intentionally leaking the  obj
    //  as a hack to avoid blocking main thread, and adding annotation to
    //  supress static code inspection warnings

    // coverity[leaked_storage]
    return createStringError(inconvertibleErrorCode(),
                             "Timed out trying to get messages from the " +
                                 m_other_endpoint_name);
  delete future;
  return json::parse(*line);
}

Error FifoFileIO::SendJSON(const json::Value &json,
                           std::chrono::milliseconds timeout) {
  bool done = false;
  std::future<void> *future =
      new std::future<void>(std::async(std::launch::async, [&]() {
        m_io->WriteLine(JSONToString(json));
        done = true;
      }));
  if (future->wait_for(timeout) == std::future_status::timeout || !done) {
    // Indeed this is a leak, but it's intentional. "future" obj destructor will
    // block on waiting for the worker thread to join. And the worker thread
    // might be stuck in blocking I/O. Intentionally leaking the  obj as a hack
    // to avoid blocking main thread, and adding annotation to supress static
    // code inspection warnings"

    // coverity[leaked_storage]
    return createStringError(inconvertibleErrorCode(),
                             "Timed out trying to send messages to the " +
                                 m_other_endpoint_name);
  }
  delete future;
  return Error::success();
}

} // namespace lldb_vscode
