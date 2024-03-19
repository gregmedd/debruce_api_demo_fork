#pragma once

#include <memory>
#include <functional>
#include <string>

class Transport {
public:
    typedef std::string Uuid;

    Transport(const std::string& name);

    bool is_open() const    { return open_fail_desc.empty(); }
    std::string open_error() const { return open_fail_desc; }
    size_t use_count() const { if (pImpl) return pImpl.use_count(); return 0; }
    bool is_same(const Transport& other) const { return this->pImpl == other.pImpl; }

    std::string process(const std::string& arg);

    void process_with_callable(std::function<Uuid (size_t)>);
private:
    std::string open_fail_desc;
    struct Impl;
    std::shared_ptr<Impl> pImpl;
};