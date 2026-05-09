#pragma once
#include <string>
#include <unordered_map>
#include <string_view>

namespace Prism {

struct TransparentHash {
    using is_transparent = void;
    size_t operator()(std::string_view sv) const {
        return std::hash<std::string_view>{}(sv);
    }
};

class PRISM_API LanguageManager {
public:
    static LanguageManager& Get();

    void LoadLanguage(const std::string& filepath);
    const char* Translate(const char* text);

    const std::string& GetCurrentLanguage() const { return m_CurrentLanguage; }

    void ClearCache() { m_ResultCache.clear(); }

private:
    LanguageManager() = default;

    std::unordered_map<std::string, std::string, TransparentHash, std::equal_to<>> m_Translations;
    std::unordered_map<const char*, const char*> m_ResultCache;
    std::string m_CurrentLanguage = "en-US";
};

}

#define TR(text) Prism::LanguageManager::Get().Translate(text)
