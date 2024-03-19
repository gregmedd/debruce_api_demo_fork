#pragma once

#include <memory>
#include <functional>
#include <string>

class Transport {
public:
    struct Impl;
    typedef std::string Uuid;

    Transport(const std::string& name);

    bool is_open() const    { return open_fail_desc.empty(); }
    std::string open_error() const { return open_fail_desc; }
    size_t use_count() const { if (pImpl) return pImpl.use_count(); return 0; }
    bool is_same(const Transport& other) const { return this->pImpl == other.pImpl; }

    std::string process(const std::string& arg);

    void process_with_callable(std::function<Uuid (size_t)>);

    static void force_inst();
private:
    std::string open_fail_desc;
    std::shared_ptr<Impl> pImpl;
};

struct Transport::Impl {
    virtual std::string process(const std::string& arg) = 0;
    virtual void process_with_callable(std::function<Transport::Uuid (size_t)> func) = 0;
};