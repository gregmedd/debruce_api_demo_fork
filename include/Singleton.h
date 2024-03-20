#ifndef SINGLETON_H
#define SINGLETON_H

#include <any>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <variant>

////////////////////////////////////////////////////////////////////////////////
/// @brief Contains the singleton wrapper and all optional checking mechanisms.
namespace Singleton {

////////////////////////////////////////////////////////////////////////////////
/// @brief Enables checking that singletons are valid at construction time.
///
/// To enable checking for your class, inherit from Check and implement the
/// virtual methods. Then wrap your class in the Wrapper.
struct Check {
    ////////////////////////////////////////////////////////////////////////////
    /// @returns True if this instance of this object was constructed and
    ///          initialized successfully.
    virtual bool instanceOk() const = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Enables result reporting when singletons are constructed.
///
/// @note This cannot be used directly. Instead, inherit from CheckWithResult.
struct Result {
    virtual std::any instanceResult() const = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Enables result reporting when singletons are constructed.
///
/// @tparam ResultT Type of the result value returned. This could be a numeric
///         code or an enum class, or any other copyable value that will work
///         in a std::any.
template<typename ResultT>
struct CheckWithResult : public Check, public Result {
    ////////////////////////////////////////////////////////////////////////////
    /// @returns Shortcut for getting values out of std::any
    ///
    /// This would be used from the derived class. For example, given
    ///     struct Foo : public CheckWithResult<int> {...};
    ///     using FooSingleton = Singleton::Wrapper<Foo>;
    ///
    /// Callers could get the singleton and, in the event of a failed init,
    /// check and return the result code like this:
    ///
    ///     auto result = FooSingleton::instance();
    ///     if (std::has_variant<std::any>(result)) {
    ///         return FooSingleton::getResultValue(result);
    ///     }
    static ResultT const getResultValue(std::any const& result) {
        return std::any_cast<ResultT>(result);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @returns The result code for a failed initialization.
    ///
    /// @remarks Only called when instanceOk() returns false.
    virtual ResultT instanceResultValue() const = 0;

private:
    ////////////////////////////////////////////////////////////////////////////
    /// @returns The output of instanceResultValue() wrapped as a std::any.
    std::any instanceResult() const final { return instanceResultValue(); }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Wraps a given class T with Singleton functionality.
///
/// @tparam T Class to wrap
/// @tparam keepalive When true, this class will hold a static shared_ptr handle
///         internally to extend the life of the singleton instance until exit.
///         This will occur regardless of the number of instances held outside
///         of the Wrapper. Setting this to false (the default) allows for more
///         direct control over the lifecycle of the wrapped singleton, and
///         makes unit testing of singletons easier.
///
/// @note Developers making use of this class are responsible for ensuring their
///       T is non-copyable and that access is performed through the singleton
///       handles.
template<typename T, bool keepalive = false>
class Wrapper : public T {
public:
    /// @brief The wrapped type
    using WrapT = T;
    /// @brief Handle to the singleton instance.
    ///
    /// When keepalive is false, at least one must be held outside Wrapper to
    /// prevent the singleton from being destructed.
    using Handle = std::shared_ptr<T>;

private:
    template<typename... Args>
    Wrapper(Args&&... args) : T(std::forward<Args>(args)...) { }

    Wrapper(Wrapper const&) = delete;
    Wrapper& operator=(Wrapper const&) = delete;

    static inline Handle keepalive_handle_;
    static inline typename Handle::weak_type weak_instance_handle_;
    static inline std::mutex instance_lock_;
    using MaybeLock = std::optional<std::unique_lock<std::mutex>>;

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Internal shared implementation for all versions of instance().
    ///
    /// @returns A handle and, if a new instance was constructed, instance_lock_
    ///          locked in a unique_lock.
    template<typename... Args>
    static std::pair<Handle, MaybeLock> instance_impl(Args&&... args) {
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
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get a handle to the wrapped singleton with no extra checking.
    ///
    /// Forwards all arguments to T's constructor.
    ///
    /// @remarks Don't worry about the template parameters. Just call this as
    ///          `auto x = instance(some, args)`
    template<typename... Args, typename T2 = T,
        std::enable_if_t<!std::is_base_of_v<Check, T2>, bool> = true>
    [[nodiscard]] static Handle instance(Args&&... args) {
        auto instance_and_lock = instance_impl(std::forward<Args>(args)...);
        auto& instance_handle{instance_and_lock.first};
        auto& maybe_lock{instance_and_lock.second};

        // If a lock was handed to us, that means a new instance was created
        if (maybe_lock) {
            weak_instance_handle_ = instance_handle;
            if constexpr (keepalive) keepalive_handle_ = instance_handle;
        }

        return instance_handle;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get a handle to the wrapped singleton, checking for success on
    ///        construction.
    ///
    /// Forwards all arguments to T's constructor. If T's instanceOk() returns
    /// false, this will discard that instance of T and return nullptr.
    ///
    /// @remarks Don't worry about the template parameters. Just call this as
    ///          `auto x = instance(some, args)`
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
                    if constexpr (keepalive) keepalive_handle_ = instance_handle;
                } else {
                    return {};
                }
            }
        }

        return instance_handle;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get a handle to the wrapped singleton, checking for success on
    ///        construction and returning a result value for errors.
    ///
    /// Forwards all arguments to T's constructor. If T's instanceOk() returns
    /// false, this will discard that instance of T and return whatever T
    /// provides via instaceResultValue() as a std::any.
    ///
    /// @remarks Don't worry about the template parameters. Just call this as
    ///          `auto x = instance(some, args)`
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
                    if constexpr (keepalive) keepalive_handle_ = instance_handle;
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
