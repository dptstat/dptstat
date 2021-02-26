#pragma once

#include <array>
#include <string_view>
#include <windows.h>
#include <wininet.h>

namespace toolbox {
namespace http {
using handle_t = HINTERNET;
using dword_t = DWORD;
using dword_ptr_t = DWORD_PTR;
using port_t = INTERNET_PORT;
namespace internal {
class internet_handle {
public:
    internet_handle(handle_t handle) : handle{handle} {}
    internet_handle(internet_handle&& other) {
        handle = other.handle;
        other.handle = 0;
    }
    internet_handle& operator=(internet_handle&& other) {
        handle = other.handle;
        other.handle = 0;
        return *this;
    }
    virtual ~internet_handle() {
        close();
    }
    virtual operator handle_t() const {
        return handle;
    }
    virtual operator handle_t() {
        return handle;
    }
    virtual void close() {
        InternetCloseHandle(handle);
        handle = 0;
    }
private:
    internet_handle(const internet_handle&) = delete;
    internet_handle& operator=(const internet_handle&) = delete;
protected:
    handle_t handle;
};
class connection : public internal::internet_handle {
public:
    static connection& instance() {
        static connection h{};
        return h;
    }
    handle_t open(std::string_view&& agent, dword_t access_type, std::string_view&& proxy, std::string_view&& proxy_bypass, dword_t flags) {
        if (!handle) {
            handle = InternetOpen(agent.data(), access_type, proxy.data(), proxy_bypass.data(), flags);
        }
        return handle;
    }
    handle_t open() {
        return open("Internet", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    }
    operator handle_t() const override {
        return instance().open();
    }
    operator handle_t() override {
        return instance().open();
    }
private:
    connection() : internal::internet_handle{0} {}
};
} // namespace internal
class request : public internal::internet_handle {
public:
    enum class type_t {GET, POST};
    static constexpr const char* type(type_t t) {
        switch(t) {
        case type_t::GET: return "GET";
        case type_t::POST: return "POST";
        default: return "";
        }
    }
    request(type_t t, handle_t session_handle, std::string_view&& object, std::string_view&& version, std::string_view&& referrer, const char** accept_types, dword_t flags, dword_ptr_t context) :
        internal::internet_handle{HttpOpenRequest(session_handle, type(t), object.data(), version.data(), referrer.data(), accept_types, flags, context)} {}
    request(type_t t, handle_t session_handle, std::string_view&& object) :
        request{t, session_handle, std::move(object), "HTTP/1.1", nullptr, nullptr, INTERNET_FLAG_RELOAD | INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI, 0} {}
    bool send(const char* headers, dword_t headers_length, void* optional, dword_t optional_length) {
        return static_cast<bool>(HttpSendRequest(handle, headers, headers_length, optional, optional_length));
    }
    bool send() {
        return send(nullptr, 0, nullptr, 0);
    }
    template <std::size_t ReadBufferSize = 512, typename StreamBuffer> void
    read_to_stream(StreamBuffer&& buffer) {
        static std::array<char, ReadBufferSize> read_buffer;
        dword_t n_bytes{0};
        while(InternetReadFile(handle, read_buffer.data(), ReadBufferSize, &n_bytes) && n_bytes) {
            buffer.write(read_buffer.data(), n_bytes);
        }
    }
    template <std::size_t ReadBufferSize = 512, typename DynamicBuffer> std::size_t
    read_to_dynamic_buffer(DynamicBuffer&& dynamic_buffer) {
        dword_t n_total_bytes{0};
        dword_t n_bytes{0};
        if (dynamic_buffer.size() < ReadBufferSize) {
            dynamic_buffer.resize(ReadBufferSize);
        }
        std::size_t dynamic_buffer_size = dynamic_buffer.size();
        auto data_address = dynamic_buffer.data();
        while(InternetReadFile(handle, data_address, ReadBufferSize, &n_bytes) && n_bytes) {
            n_total_bytes += n_bytes;
            if (dynamic_buffer_size <= (n_total_bytes + ReadBufferSize)) {
                dynamic_buffer_size = dynamic_buffer_size * 2;
                dynamic_buffer.resize(dynamic_buffer_size);
            }
            data_address = dynamic_buffer.data() + (n_total_bytes * sizeof(char));
        }
        return n_total_bytes;
    }
    template <std::size_t ReadBufferSize = 512, typename AllocatedBuffer> std::size_t
    read_to_allocated_buffer(AllocatedBuffer&& allocated_buffer) {
        dword_t n_total_bytes{0};
        dword_t n_bytes{0};
        auto data_address = allocated_buffer.data();
        while((n_total_bytes < (allocated_buffer.size() - ReadBufferSize)) && (InternetReadFile(handle, data_address, ReadBufferSize, &n_bytes)) && (n_bytes)) {
            data_address = data_address + (n_bytes * sizeof(char));
            n_total_bytes += n_bytes;
        }
        return n_total_bytes;
    }
};
class session : public internal::internet_handle {
public:
    session(std::string_view&& host, port_t port, std::string_view&& user , std::string_view&& pass, dword_t service, dword_t flags, dword_ptr_t context) :
        internal::internet_handle{InternetConnect(internal::connection::instance(), host.data(), port, user.data(), pass.data(), service, flags, context)} {}
    session(std::string_view&& host) :
        session{std::move(host), INTERNET_DEFAULT_HTTP_PORT, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0} {}
    request get(std::string_view&& object) {
        return std::move(request(request::type_t::GET, *this, std::move(object)));
    }
};
inline void open_connection(std::string_view&& agent, dword_t access_type, std::string_view&& proxy, std::string_view&& proxy_bypass, dword_t flags) {
    internal::connection::instance().open(std::move(agent), access_type, std::move(proxy), std::move(proxy_bypass), flags);
}
inline void open_connection() {
    internal::connection::instance().open();
}
inline void close_connection() {
    internal::connection::instance().close();
}
} // namespace http
} // namespace toolbox
