#pragma once

namespace Prism
{
    template<typename Signature>
    class Function;

    template<typename R, typename... Args>
    class Function<R(Args...)>
    {
    private:
        struct ICallable
        {
            virtual ~ICallable() = default;
            virtual R Invoke(Args... args) = 0;
            virtual ICallable* Clone() const = 0;
        };
        template<typename F>
        struct Callable : ICallable
        {
            F func;
            Callable(F&& f) : func(std::forward<F>(f)) {}
            R Invoke(Args... args) override
            {
                return func(std::forward<Args>(args)...);
            }
            ICallable* Clone() const override
            {
                return new Callable<F>(func);
            }
        };
    public:
        Function() = default;
        template<typename F>
        Function(F&& f) : callable(new Callable<F>(std::forward<F>(f))) {}
        Function(const Function& other) : callable(other.callable ? other.callable->Clone() : nullptr) {}
        Function(Function&& other) noexcept : callable(other.callable) { other.callable = nullptr; }
        Function& operator=(const Function& other) 
        {
            if (this != &other)
            {
                delete callable;
                callable = other.callable ? other.callable->Clone() : nullptr;
            }
            return *this;
        }
        Function& operator=(Function&& other) noexcept
        {
            if (this != &other)
            {
                delete callable;
                callable = other.callable;
                other.callable = nullptr;
            }
            return *this;
        }
        ~Function() { delete callable; }
        R operator()(Args... args) const { return callable->Invoke(std::forward<Args>(args)...); }
        explicit operator bool() const { return callable != nullptr; }
    private:
        ICallable* callable = nullptr;
    };
}
