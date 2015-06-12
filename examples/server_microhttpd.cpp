#include <microhttpd.h>

#include <memory>
#include <string>
#include <cstring>
#include <iostream>

using Port = std::uint16_t;
using std::strcmp;
using std::cout;
using std::endl;
using StringUniquePtr = std::unique_ptr<std::string>;

enum class Method {
    GET,
    POST,
    PUT,
    PATCH,
    DELETE,
    HEAD
};

static int send_response(struct MHD_Connection* connection,
        unsigned status, const std::string& message) {
    struct MHD_Response* mhd_response = MHD_create_response_from_buffer(
            message.length(),
            const_cast<char*>(message.c_str()),
            MHD_RESPMEM_MUST_COPY);

    if (nullptr == mhd_response) {
        cout << "Cannot create response" << endl;
        return MHD_NO;
    }

    MHD_add_response_header(mhd_response, MHD_HTTP_HEADER_CONTENT_TYPE,
        "application/json");

    /* Response to client */
    int ret = MHD_queue_response(connection, status, mhd_response);
    MHD_destroy_response(mhd_response);

    return ret;
}

static int request_with_data(Method http_method,
        void* cls, struct MHD_Connection* connection,
        const char* url, const char* method, const char* version,
        const char* upload_data, size_t* upload_data_size, void** con_cls)
{
    (void)http_method;
    (void)cls;
    (void)url;
    (void)method;
    (void)version;

    StringUniquePtr data{static_cast<std::string*>(*con_cls)};

    if (nullptr == data) {
        data.reset(new (std::nothrow) std::string);
        if (nullptr == data) {
            return MHD_NO;
        }
        *con_cls = data.release();
        return MHD_YES;
    }

    if (0 != *upload_data_size) {
        data->append(upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }

    *con_cls = nullptr;

    cout << *data << endl;

    return send_response(connection, MHD_HTTP_OK, "Data received\n");
}

static int request_no_data(Method http_method,
        void* cls, struct MHD_Connection* connection,
        const char* url, const char* method, const char* version,
        const char* upload_data, size_t* upload_data_size, void** con_cls)
{
    (void)http_method;
    (void)cls;
    (void)url;
    (void)method;
    (void)version;
    (void)upload_data;

    if (0 != *upload_data_size) {
        /* Upload data !? */
        return MHD_NO;
    }
    *con_cls = nullptr;

    return send_response(connection, MHD_HTTP_OK, "No data\n");
}

static int access_handler_callback(void* cls, struct MHD_Connection *connection,
    const char* url, const char* method, const char* version,
    const char* upload_data, size_t* upload_data_size, void** con_cls) {

    cout << "Method: " << method << endl;

    if (0 == strcmp(method, "GET")) {
        return request_no_data(Method::GET,
                cls, connection, url, method, version,
                upload_data, upload_data_size, con_cls);
    } else if (0 == strcmp(method, "POST")) {
        return request_with_data(Method::POST,
                cls, connection, url, method, version,
                upload_data, upload_data_size, con_cls);
    } else if (0 == strcmp(method, "PUT")) {
        return request_with_data(Method::PUT,
                cls, connection, url, method, version,
                upload_data, upload_data_size, con_cls);
    } else if (0 == strcmp(method, "PATCH")) {
        return request_with_data(Method::PATCH,
                cls, connection, url, method, version,
                upload_data, upload_data_size, con_cls);
    } else if (0 == strcmp(method, "DELETE")) {
        return request_no_data(Method::DELETE,
                cls, connection, url, method, version,
                upload_data, upload_data_size, con_cls);
    } else if (0 == strcmp(method, "HEAD")) {
        return request_no_data(Method::HEAD,
                cls, connection, url, method, version,
                upload_data, upload_data_size, con_cls);
    }

    return send_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "No supported");
}

int main(int argc, char* argv[]) {
    Port port = 6666;

    if (argc > 1) {
        port = Port(std::stoi(argv[1]));
    }

    struct MHD_Daemon* mhd = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
            port, nullptr, nullptr,
            access_handler_callback, nullptr,
            MHD_OPTION_END);
    if (nullptr == mhd) {
        cout << "Cannot start daemon" << endl;
        return 1;
    }

    cout << "Daemon started..." << endl;
    std::getchar();

    MHD_stop_daemon(mhd);
    cout << "Daemon stopped" << endl;

    return 0;
}
