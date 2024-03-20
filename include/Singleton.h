#ifndef SINGLETON_H
#define SINGLETON_H

#include <any>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <variant>

namespace Singleton {

struct Check {
    virtual bool instanceOk() const = 0;
};

struct Result {
    virtual std::any instanceResult() const { return {}; };
};

template<typename ResultT>
struct CheckWithResult : public Check, public Result {
    static ResultT const getResultValue(std::any const& result) {
        return std::any_cast<ResultT>(result);
    }

    virtual ResultT instanceResultValue() const = 0;

private:
    std::any instanceResult() const final { return instanceResultValue(); }
};

template<typename T>
class Wrapper : public T {
    template<typename... Args>
    Wrapper(Args&&... args) : T(std::forward<Args>(args)...) { }

    Wrapper(Wrapper const&) = delete;
    Wrapper& operator=(Wrapper const&) = delete;

    static inline std::weak_ptr<T> weak_instance_handle_;
    static inline std::mutex instance_lock_;
    using MaybeLock = std::optional<std::unique_lock<std::mutex>>;

    template<typename... Args>
    static std::pair<std::shared_ptr<T>, MaybeLock> instance_impl(Args&&... args) {
        if (auto instance_handle = weak_instance_handle_.lock()) {
            return {instance_handle, MaybeLock()};
        } else {
            std::unique_lock lock(instance_lock_);

            // Avoid race conditions by checking again for instance validity
            // after the mutex is taken.
            if (instance_handle = weak_instance_handle_.lock()) {
                return {instance_handle, MaybeLock()};
            }

            instance_handle = std::make_shared<T>(std::forward<Args>(args)...);
            return {instance_handle, std::move(lock)};
        }
    }

public:
    using WrapT = T;
    using Handle = std::shared_ptr<T>;

    template<typename... Args, typename T2 = T,
        std::enable_if_t<!std::is_base_of_v<Check, T2>, bool> = true>
    [[nodiscard]] static Handle instance(Args&&... args) {
        auto instance_and_lock = instance_impl(std::forward<Args>(args)...);
        auto& instance_handle{instance_and_lock.first};
        auto& maybe_lock{instance_and_lock.second};

        // If a lock was handed to us, that means a new instance was created
        if (maybe_lock) {
            weak_instance_handle_ = instance_handle;
        }

        return instance_handle;
    }

    template<typename... Args, typename T2 = T,
        std::enable_if_t<
            std::is_base_of_v<Check, T2> && !std::is_base_of_v<Result, T2>,
            bool> = true>
    [[nodiscard]] static Handle instance(Args&&... args) {
        auto instance_and_lock = instance_impl(std::forward<Args>(args)...);
        auto& instance_handle{instance_and_lock.first};
        auto& maybe_lock{instance_and_lock.second};

        // If a lock was handed to us, that means a new instance was created
        if (maybe_lock) {
            if (instance_handle) {
                auto check = dynamic_cast<Check*>(instance_handle.get());
                if (check && check->instanceOk()) {
                    weak_instance_handle_ = instance_handle;
                } else {
                    return {};
                }
            }
        }

        return instance_handle;
    }

    template<typename... Args, typename T2 = T,
        std::enable_if_t<
            std::is_base_of_v<Check, T2> && std::is_base_of_v<Result, T2>,
            bool> = true>
    [[nodiscard]] static std::variant<Handle, std::any> instance(Args&&... args) {
        auto instance_and_lock = instance_impl(std::forward<Args>(args)...);
        auto& instance_handle{instance_and_lock.first};
        auto& maybe_lock{instance_and_lock.second};

        // If a lock was handed to us, that means a new instance was created
        if (maybe_lock) {
            if (instance_handle) {
                auto check = dynamic_cast<Check*>(instance_handle.get());
                if (check && check->instanceOk()) {
                    weak_instance_handle_ = instance_handle;
                } else {
                    auto result = dynamic_cast<Result*>(instance_handle.get());
                    if (result) {
                        return result->instanceResult();
                    } else {
                        return {};
                    }
                }
            }
        }

        return instance_handle;
    }
};

} // namespace Singleton

#endif // SINGLETON_H
