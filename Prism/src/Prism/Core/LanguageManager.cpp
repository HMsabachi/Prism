#include "prpch.h"
#include "LanguageManager.h"

#include <yaml-cpp/yaml.h>

namespace Prism {

LanguageManager& LanguageManager::Get()
{
    static LanguageManager instance;
    return instance;
}

void LanguageManager::LoadLanguage(const std::string& filepath)
{
    try
    {
        YAML::Node data = YAML::LoadFile(filepath);
        m_CurrentLanguage = data["language"].as<std::string>("en-US");

        m_Translations.clear();
        auto translations = data["translations"];
        if (translations)
        {
            for (auto it = translations.begin(); it != translations.end(); ++it)
            {
                std::string key = it->first.as<std::string>();
                std::string value = it->second.as<std::string>();
                m_Translations.emplace(std::move(key), std::move(value));
            }
        }

        m_ResultCache.clear();
        PR_CORE_INFO("Loaded language '{0}' from {1}", m_CurrentLanguage, filepath);
    }
    catch (const std::exception& e)
    {
        PR_CORE_ERROR("Failed to load language file '{0}': {1}", filepath, e.what());
    }
}

const char* LanguageManager::Translate(const char* text)
{
    auto cacheIt = m_ResultCache.find(text);
    if (cacheIt != m_ResultCache.end())
        return cacheIt->second;

    auto it = m_Translations.find(text);
    const char* result = (it != m_Translations.end()) ? it->second.c_str() : text;

    m_ResultCache[text] = result;
    return result;
}

}
