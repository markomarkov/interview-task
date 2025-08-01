#include <iostream>

#include <nlohmann/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <openssl/ssl.h>

#include "../config.h"
#include "okx_client.h"

OKXClient::OKXClient(const std::string& certificate_file_path) : cert_file_path(certificate_file_path) {}

nlohmann::json OKXClient::fetch_tickers(std::string& error) {
    // FUTURES
    std::string error_futures;
    nlohmann::json futures_data = fetch_json_https("www.okx.com", "443", "/api/v5/public/instruments?instType=FUTURES", error_futures);
    // SWAP
    std::string error_swap;
    nlohmann::json swap_data = fetch_json_https("www.okx.com", "443", "/api/v5/public/instruments?instType=SWAP", error_swap);

    nlohmann::json result;
    result["code"] = "0";
    result["msg"] = "";
    result["data"] = nlohmann::json::array();
    if (futures_data.contains("data") && futures_data["data"].is_array()) {
        for (const auto& item : futures_data["data"]) {
            result["data"].push_back(item);
        }
    }
    if (swap_data.contains("data") && swap_data["data"].is_array()) {
        for (const auto& item : swap_data["data"]) {
            result["data"].push_back(item);
        }
    }

    if (!error_futures.empty() && !error_swap.empty()) {
        error = "FUTURES: " + error_futures + "; SWAP: " + error_swap;
    } else if (!error_futures.empty()) {
        error = "FUTURES: " + error_futures;
    } else if (!error_swap.empty()) {
        error = "SWAP: " + error_swap;
    } else {
        error.clear();
    }
    return result;
}

nlohmann::json OKXClient::fetch_orderbook(const std::string& instId, std::string& error) {
    return fetch_json_https("www.okx.com", "443", "/api/v5/market/books-full?instId=" + instId + "&sz=" + std::to_string(MAX_ORDERBOOK_ROWS), error);
}

nlohmann::json OKXClient::fetch_json_https(const std::string& host, const std::string& port, const std::string& target, std::string& error_out) {
    try {
        boost::asio::io_context ioContext;
        boost::asio::ssl::context sslContext(boost::asio::ssl::context::tls_client);
        sslContext.set_default_verify_paths();
        sslContext.load_verify_file(cert_file_path);
        boost::asio::ssl::stream<boost::beast::tcp_stream> stream(ioContext, sslContext);
        stream.set_verify_mode(boost::asio::ssl::verify_peer);
        boost::asio::ip::tcp::resolver resolver(ioContext);
        auto const results = resolver.resolve(host, port);
        boost::beast::get_lowest_layer(stream).connect(results);
        if(!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
            throw boost::beast::system_error(
                boost::beast::error_code(static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()),
                "Failed to set SNI Hostname");
        stream.handshake(boost::asio::ssl::stream_base::client);
        boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, target, 11};
        req.set(boost::beast::http::field::host, host);
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        boost::beast::http::write(stream, req);
        boost::beast::flat_buffer buffer;
        boost::beast::http::response<boost::beast::http::string_body> res;
        boost::beast::http::read(stream, buffer, res);
        boost::beast::error_code connectionErrorCode;
        stream.shutdown(connectionErrorCode);
        error_out.clear();
        return nlohmann::json::parse(res.body());
    } catch (std::exception const& ex) {
        error_out = ex.what();
        return nlohmann::json();
    }
}
