#pragma once
#include <vector>
#include <functional>
#include <algorithm>

namespace Prism
{
    template<typename... Args>
    class Delegate
    {
    public:
        using FuncType = std::function<void(Args...)>;
        using Token = uint64_t;
        struct Invokeer
        {
            FuncType Func;
            Token Id;
        };
        Token Add(FuncType func)
        {
            Token currentToken = m_NextToken++;
            m_Functions.push_back({ std::move(func), currentToken });
            return currentToken;
        }
        void Remove(Token token)
        {
            auto it = std::remove_if(m_Functions.begin(), m_Functions.end(),
                [token](const Invokeer& invokeer) { return invokeer.Id == token; });
            m_Functions.erase(it, m_Functions.end());
        }
        void Clear()
        {
            m_Functions.clear();
        }
        void Invoke(Args... args)
        {
            if (m_Functions.empty()) return;
            auto functionsCopy = m_Functions;
            for (const auto& invokeer : functionsCopy)
            {
                if (invokeer.Func)
                    invokeer.Func(args...);
            }
        }
        void operator()(Args... args)
        {
            Invoke(args...);
        }
    private:
        std::vector<Invokeer> m_Functions;
        Token m_NextToken = 1;
    };
}
