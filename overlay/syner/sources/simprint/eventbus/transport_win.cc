// Copyright 2024 Simprint. All rights reserved.
// EventBus 传输层 - Windows Named Pipe 客户端实现

#include "simprint/eventbus/transport.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/sequenced_task_runner.h"
#include "base/threading/thread.h"
#include "build/build_config.h"

#if BUILDFLAG(IS_WIN)

// 例行流程不输出，仅真实错误在 NotifyError 中 LOG(ERROR)；需要调试时用 --v=1
#define SIMPRINT_LOG(msg) VLOG(1) << "[Simprint][Transport] " << msg

namespace simprint {
namespace eventbus {

namespace {

constexpr size_t kReadBufferSize = 64 * 1024;  // 64KB
constexpr DWORD kConnectTimeoutMs = 5000;      // 5 秒连接超时

}  // namespace

NamedPipeTransport::NamedPipeTransport(const std::string& env_id,
                                       TransportDelegate* delegate)
    : env_id_(env_id),
      delegate_(delegate),
      main_task_runner_(base::SequencedTaskRunner::GetCurrentDefault()) {
  DCHECK(delegate_);
  DETACH_FROM_SEQUENCE(main_sequence_checker_);

  read_buffer_.reserve(kReadBufferSize);

  io_thread_ = std::make_unique<base::Thread>("EventBusPipeIO");
  base::Thread::Options options;
  options.message_pump_type = base::MessagePumpType::IO;
  io_thread_->StartWithOptions(std::move(options));
}

NamedPipeTransport::~NamedPipeTransport() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(main_sequence_checker_);
  Disconnect();
  io_thread_->Stop();
}

std::wstring NamedPipeTransport::GetPipePath() const {
  // 格式: \\.\pipe\simprint_<env_id>
  return L"\\\\.\\pipe\\simprint_" + base::UTF8ToWide(env_id_);
}

void NamedPipeTransport::Connect() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(main_sequence_checker_);

  if (state_ == TransportState::kConnecting ||
      state_ == TransportState::kConnected) {
    return;
  }

  state_ = TransportState::kConnecting;

  io_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&NamedPipeTransport::DoConnect,
                     base::Unretained(this)));
}

void NamedPipeTransport::DoConnect() {
  std::wstring pipe_path = GetPipePath();
  std::string pipe_path_utf8 = base::WideToUTF8(pipe_path);

  SIMPRINT_LOG("DoConnect - pipe: " << pipe_path_utf8);

  // 等待管道可用
  if (!WaitNamedPipeW(pipe_path.c_str(), kConnectTimeoutMs)) {
    DWORD error = GetLastError();
    if (error != ERROR_SEM_TIMEOUT) {
      NotifyError("WaitNamedPipe failed: " + std::to_string(error));
      return;
    }
    // 超时，管道可能还不存在
    NotifyError("Pipe not available (timeout)");
    return;
  }

  // 打开管道
  HANDLE handle = CreateFileW(
      pipe_path.c_str(),
      GENERIC_READ | GENERIC_WRITE,
      0,              // 不共享
      nullptr,        // 默认安全属性
      OPEN_EXISTING,  // 必须已存在
      FILE_FLAG_OVERLAPPED,  // 异步 IO
      nullptr);       // 无模板

  if (handle == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();
    NotifyError("CreateFile failed: " + std::to_string(error));
    return;
  }

  // 设置管道模式为消息模式（如果服务端是消息模式）
  DWORD mode = PIPE_READMODE_BYTE;
  if (!SetNamedPipeHandleState(handle, &mode, nullptr, nullptr)) {
    DWORD error = GetLastError();
    CloseHandle(handle);
    NotifyError("SetNamedPipeHandleState failed: " + std::to_string(error));
    return;
  }

  pipe_handle_ = handle;
  SIMPRINT_LOG("Pipe opened successfully, calling NotifyConnected");
  NotifyConnected();

  // 启动异步读取
  SIMPRINT_LOG("Starting async read");
  ScheduleRead();
}

void NamedPipeTransport::Disconnect() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(main_sequence_checker_);

  if (state_ == TransportState::kDisconnected) {
    return;
  }

  io_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&NamedPipeTransport::DoDisconnect,
                     base::Unretained(this)));
}

void NamedPipeTransport::DoDisconnect() {
  if (pipe_handle_ != INVALID_HANDLE_VALUE) {
    // 取消所有挂起的 IO 操作
    CancelIoEx(pipe_handle_, nullptr);
    CloseHandle(pipe_handle_);
    pipe_handle_ = INVALID_HANDLE_VALUE;
  }

  NotifyDisconnected();
}

bool NamedPipeTransport::Send(const Message& message) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(main_sequence_checker_);

  SIMPRINT_LOG("Send called, state=" << static_cast<int>(state_));

  if (state_ != TransportState::kConnected) {
    SIMPRINT_LOG("Send: not connected, state=" << static_cast<int>(state_));
    return false;
  }

  std::vector<uint8_t> data = message.Encode();
  SIMPRINT_LOG("Send: encoded message, size=" << data.size());

  bool posted = io_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&NamedPipeTransport::DoSend,
                     base::Unretained(this), std::move(data)));
  
  SIMPRINT_LOG("Send: PostTask result=" << posted);

  return posted;
}

void NamedPipeTransport::DoSend(std::vector<uint8_t> data) {
  SIMPRINT_LOG("DoSend called, data_size=" << data.size() << ", pipe_handle=" << (pipe_handle_ != INVALID_HANDLE_VALUE ? "valid" : "invalid"));

  if (pipe_handle_ == INVALID_HANDLE_VALUE) {
    SIMPRINT_LOG("DoSend: pipe_handle_ is INVALID, skipping");
    return;
  }

  OVERLAPPED overlapped = {};
  overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  if (!overlapped.hEvent) {
    NotifyError("CreateEvent failed for send");
    return;
  }

  DWORD bytes_written = 0;
  SIMPRINT_LOG("DoSend: Calling WriteFile...");
  BOOL result = WriteFile(pipe_handle_, data.data(),
                         static_cast<DWORD>(data.size()),
                         &bytes_written, &overlapped);

  SIMPRINT_LOG("DoSend: WriteFile returned " << result << ", bytes_written=" << bytes_written);

  if (!result) {
    DWORD error = GetLastError();
    SIMPRINT_LOG("DoSend: WriteFile error=" << error);
    if (error == ERROR_IO_PENDING) {
      // 等待写入完成
      SIMPRINT_LOG("DoSend: Waiting for IO completion...");
      if (WaitForSingleObject(overlapped.hEvent, INFINITE) == WAIT_OBJECT_0) {
        GetOverlappedResult(pipe_handle_, &overlapped, &bytes_written, FALSE);
        SIMPRINT_LOG("DoSend: IO completed, bytes_written=" << bytes_written);
      }
    } else {
      CloseHandle(overlapped.hEvent);
      NotifyError("WriteFile failed: " + std::to_string(error));
      DoDisconnect();
      return;
    }
  }

  CloseHandle(overlapped.hEvent);

  SIMPRINT_LOG("DoSend completed, sent " << bytes_written << " bytes");
}

// 调度一次异步读取
void NamedPipeTransport::ScheduleRead() {
  if (pipe_handle_ == INVALID_HANDLE_VALUE) {
    return;
  }
  // 延迟 10ms 后执行 DoRead，让其他任务有机会先执行
  io_thread_->task_runner()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&NamedPipeTransport::DoRead, base::Unretained(this)),
      base::Milliseconds(10));
}

void NamedPipeTransport::DoRead() {
  if (pipe_handle_ == INVALID_HANDLE_VALUE) {
    return;
  }

  std::vector<uint8_t> buffer(kReadBufferSize);
  DWORD bytes_read = 0;
  OVERLAPPED overlapped = {};
  overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

  if (!overlapped.hEvent) {
    NotifyError("CreateEvent failed for read");
    return;
  }

  BOOL result = ReadFile(pipe_handle_, buffer.data(),
                         static_cast<DWORD>(buffer.size()),
                         &bytes_read, &overlapped);

  if (!result) {
    DWORD error = GetLastError();
    if (error == ERROR_IO_PENDING) {
      // 等待读取完成，但使用超时
      DWORD wait_result = WaitForSingleObject(overlapped.hEvent, 100);
      if (wait_result == WAIT_OBJECT_0) {
        if (!GetOverlappedResult(pipe_handle_, &overlapped, &bytes_read, FALSE)) {
          error = GetLastError();
          CloseHandle(overlapped.hEvent);
          if (error == ERROR_OPERATION_ABORTED) {
            return;
          }
          NotifyError("GetOverlappedResult failed: " + std::to_string(error));
          return;
        }
      } else if (wait_result == WAIT_TIMEOUT) {
        // 超时，取消 IO 并重新调度
        CancelIo(pipe_handle_);
        CloseHandle(overlapped.hEvent);
        ScheduleRead();
        return;
      } else {
        CloseHandle(overlapped.hEvent);
        NotifyError("WaitForSingleObject failed");
        return;
      }
    } else if (error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED) {
      CloseHandle(overlapped.hEvent);
      NotifyDisconnected();
      return;
    } else {
      CloseHandle(overlapped.hEvent);
      NotifyError("ReadFile failed: " + std::to_string(error));
      return;
    }
  }

  CloseHandle(overlapped.hEvent);

  if (bytes_read > 0) {
    SIMPRINT_LOG("DoRead: Received " << bytes_read << " bytes");

    // 添加到接收缓冲区
    read_buffer_.insert(read_buffer_.end(),
                        buffer.begin(),
                        buffer.begin() + bytes_read);

    // 尝试解析消息
    while (!read_buffer_.empty()) {
      auto decode_result = Message::TryDecode(read_buffer_.data(), read_buffer_.size());
      if (!decode_result) {
        break;
      }

      auto& [message, consumed] = *decode_result;
      SIMPRINT_LOG("DoRead: Decoded message, topic=" << static_cast<int>(message.topic)
                  << ", msg_id=" << message.msg_id);

      NotifyMessageReceived(std::move(message));

      read_buffer_.erase(read_buffer_.begin(),
                         read_buffer_.begin() + consumed);
    }
  }

  // 继续调度下一次读取
  ScheduleRead();
}

TransportState NamedPipeTransport::GetState() const {
  return state_;
}

bool NamedPipeTransport::IsConnected() const {
  return state_ == TransportState::kConnected;
}

void NamedPipeTransport::NotifyConnected() {
  main_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](NamedPipeTransport* self) {
            if (self) {
              self->state_ = TransportState::kConnected;
              if (self->delegate_) {
                self->delegate_->OnConnected();
              }
            }
          },
          base::Unretained(this)));
}

void NamedPipeTransport::NotifyDisconnected() {
  main_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](NamedPipeTransport* self) {
            if (self) {
              self->state_ = TransportState::kDisconnected;
              self->pipe_handle_ = INVALID_HANDLE_VALUE;
              if (self->delegate_) {
                self->delegate_->OnDisconnected();
              }
            }
          },
          base::Unretained(this)));
}

void NamedPipeTransport::NotifyMessageReceived(Message message) {
  main_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](NamedPipeTransport* self, Message msg) {
            if (self && self->delegate_) {
              self->delegate_->OnMessageReceived(std::move(msg));
            }
          },
          base::Unretained(this), std::move(message)));
}

void NamedPipeTransport::NotifyError(const std::string& error) {
  main_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](NamedPipeTransport* self, std::string err) {
            if (self) {
              self->state_ = TransportState::kError;
              if (self->delegate_) {
                self->delegate_->OnError(err);
              }
            }
          },
          base::Unretained(this), error));
}

// 工厂函数
std::unique_ptr<Transport> CreateTransport(const std::string& env_id,
                                            TransportDelegate* delegate) {
  return std::make_unique<NamedPipeTransport>(env_id, delegate);
}

}  // namespace eventbus
}  // namespace simprint

#endif  // BUILDFLAG(IS_WIN)
