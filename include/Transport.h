#pragma once

#include <memory>
#include <functional>
#include <string>

class Transport {
public:
    typedef std::string Uuid;

    Transport(const std::string& name);

    bool is_open() const;
    std::string open_error() const;
    size_t use_count() const;
    bool is_same(const Transport& other);

    std::string process(const std::string& arg);

    void process_with_callable(std::function<Uuid (size_t)>);
private:
    std::string open_fail_desc;
    struct Impl;
    std::shared_ptr<Impl> pImpl;
};