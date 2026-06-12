#include "prpch.h"
#include "SourceManager.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace Prism::PSL
{

SourceManager::SourceManager(std::string filePath)
    : m_FilePath(std::move(filePath))
{
    std::ifstream file(m_FilePath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) return;

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    m_OwnedBuffer.resize(static_cast<size_t>(fileSize) + 1);
    if (!file.read(m_OwnedBuffer.data(), fileSize))
    {
        m_OwnedBuffer.clear();
        return;
    }
    m_OwnedBuffer[static_cast<size_t>(fileSize)] = '\0'; // Sentinel

    m_Buffer = m_OwnedBuffer.data();
    m_Size = static_cast<uint32_t>(fileSize);
    BuildLineTable();
}

SourceManager::SourceManager(const char* buffer, uint32_t size)
{
    Init(buffer, size);
}

SourceManager::SourceManager(SourceManager&& other) noexcept
    : m_Size(other.m_Size)
    , m_OwnedBuffer(std::move(other.m_OwnedBuffer))
    , m_FilePath(std::move(other.m_FilePath))
    , m_LineOffsets(std::move(other.m_LineOffsets))
{
    if (!m_OwnedBuffer.empty())
        m_Buffer = m_OwnedBuffer.data();
    else
        m_Buffer = other.m_Buffer;
    other.m_Buffer = nullptr;
    other.m_Size = 0;
}

SourceManager& SourceManager::operator=(SourceManager&& other) noexcept
{
    if (this != &other)
    {
        m_Size = other.m_Size;
        m_OwnedBuffer = std::move(other.m_OwnedBuffer);
        m_FilePath = std::move(other.m_FilePath);
        m_LineOffsets = std::move(other.m_LineOffsets);
        if (!m_OwnedBuffer.empty())
            m_Buffer = m_OwnedBuffer.data();
        else
            m_Buffer = other.m_Buffer;
        other.m_Buffer = nullptr;
        other.m_Size = 0;
    }
    return *this;
}

void SourceManager::Init(const char* buffer, uint32_t size)
{
    m_Buffer = buffer;
    m_Size = size;
    BuildLineTable();
}

void SourceManager::BuildLineTable()
{
    m_LineOffsets.clear();
    m_LineOffsets.push_back(0); // 第 0 行从 offset 0 开始

    for (uint32_t i = 0; i < m_Size; ++i)
        if (m_Buffer[i] == '\n')
            m_LineOffsets.push_back(i + 1); // 下一行从 \n 之后开始
}

uint32_t SourceManager::GetLine(uint32_t offset) const
{
    if (m_LineOffsets.empty()) return 0;
    // 二分查找：最后一个 <= offset 的换行位置
    auto it = std::upper_bound(m_LineOffsets.begin(), m_LineOffsets.end(), offset);
    return static_cast<uint32_t>(std::distance(m_LineOffsets.begin(), it) - 1);
}

uint32_t SourceManager::GetColumn(uint32_t offset) const
{
    uint32_t line = GetLine(offset);
    return offset - m_LineOffsets[line] + 1; // 列号从 1 开始
}

SourceLocation SourceManager::GetLocation(uint32_t offset) const
{
    return {
        GetLine(offset) + 1,    // 行号从 1 开始
        GetColumn(offset),
        offset,
        std::string_view(m_FilePath)
    };
}

std::string_view SourceManager::GetView(uint32_t offset, uint32_t length) const
{
    if (!m_Buffer || offset >= m_Size) return {};
    uint32_t maxLen = m_Size - offset;
    return std::string_view(m_Buffer + offset, length < maxLen ? length : maxLen);
}

} // namespace Prism::PSL
