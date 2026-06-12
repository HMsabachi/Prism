#pragma once

#include "Prism/Shader/ShaderCommon.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Prism::PSL
{

class SourceManager
{
public:
    SourceManager() = default;
    explicit SourceManager(std::string filePath);
    SourceManager(const char* buffer, uint32_t size);

    // 移动：buffer 指针需要更新到新 owner
    SourceManager(SourceManager&& other) noexcept;
    SourceManager& operator=(SourceManager&& other) noexcept;
    SourceManager(const SourceManager&) = delete;
    SourceManager& operator=(const SourceManager&) = delete;

    void Init(const char* buffer, uint32_t size);
    void SetFilePath(const std::string& path) { m_FilePath = path; }

    uint32_t GetLine(uint32_t offset) const;
    uint32_t GetColumn(uint32_t offset) const;
    SourceLocation GetLocation(uint32_t offset) const;

    std::string_view GetView(uint32_t offset, uint32_t length) const;
    const char* GetBuffer() const { return m_Buffer; }
    uint32_t GetSize() const { return m_Size; }
    const std::string& GetFilePath() const { return m_FilePath; }

    bool IsValid() const { return m_Buffer != nullptr; }

private:
    void BuildLineTable();

    const char* m_Buffer = nullptr;
    uint32_t m_Size = 0;
    std::string m_OwnedBuffer;
    std::string m_FilePath;
    std::vector<uint32_t> m_LineOffsets;
};

} // namespace Prism::PSL
