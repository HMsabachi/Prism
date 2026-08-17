#include "prpch.h"
#include "Instrumentor.h"

#include <functional>

namespace Prism
{

    Instrumentor::Instrumentor()
        : m_CurrentSession(nullptr),
        m_ProfileCount(0)
    {
    }


    Instrumentor& Instrumentor::Get()
    {
        static Instrumentor instance;
        return instance;
    }


    void Instrumentor::BeginSession(
        const std::string& name,
        const std::string& filepath)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        // 如果之前还有 session，先结束
        if (m_CurrentSession)
        {
            WriteFooter();

            m_OutputStream.close();

            delete m_CurrentSession;
            m_CurrentSession = nullptr;

            m_ProfileCount = 0;
        }

        m_OutputStream.open(filepath);

        if (!m_OutputStream.is_open())
        {
            return;
        }

        m_CurrentSession = new InstrumentationSession{ name };

        m_ProfileCount = 0;

        WriteHeader();
    }


    void Instrumentor::EndSession()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        if (!m_CurrentSession)
            return;

        WriteFooter();

        m_OutputStream.close();

        delete m_CurrentSession;
        m_CurrentSession = nullptr;

        m_ProfileCount = 0;
    }


    void Instrumentor::WriteProfile(const ProfileResult& result)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        // 没有正在进行的 Session
        if (!m_CurrentSession)
            return;

        if (!m_OutputStream.is_open())
            return;

        if (m_ProfileCount++ > 0)
            m_OutputStream << ",";

        std::string name = result.Name;

        // 防止 JSON 字符串中的双引号破坏 JSON
        std::replace(
            name.begin(),
            name.end(),
            '"',
            '\''
        );

        m_OutputStream << "{";

        m_OutputStream << "\"cat\":\"function\",";

        m_OutputStream
            << "\"dur\":"
            << (result.End - result.Start)
            << ",";

        m_OutputStream
            << "\"name\":\""
            << name
            << "\",";

        m_OutputStream << "\"ph\":\"X\",";

        m_OutputStream << "\"pid\":0,";

        m_OutputStream
            << "\"tid\":"
            << result.ThreadID
            << ",";

        m_OutputStream
            << "\"ts\":"
            << result.Start;

        m_OutputStream << "}";

        // 不建议每次 Profile 都 flush
        // m_OutputStream.flush();
    }


    void Instrumentor::WriteHeader()
    {
        m_OutputStream
            << "{\"otherData\": {},\"traceEvents\":[";

        // Header 只写一次，可以 flush
        m_OutputStream.flush();
    }


    void Instrumentor::WriteFooter()
    {
        m_OutputStream << "]}";

        m_OutputStream.flush();
    }


    InstrumentationTimer::InstrumentationTimer(
        const char* name)
        : m_Name(name),
        m_Stopped(false)
    {
        m_StartTimepoint =
            std::chrono::high_resolution_clock::now();
    }


    InstrumentationTimer::~InstrumentationTimer()
    {
        if (!m_Stopped)
            Stop();
    }


    void InstrumentationTimer::Stop()
    {
        if (m_Stopped)
            return;

        auto endTimepoint =
            std::chrono::high_resolution_clock::now();

        long long start =
            std::chrono::time_point_cast<
            std::chrono::microseconds
            >(m_StartTimepoint)
            .time_since_epoch()
            .count();

        long long end =
            std::chrono::time_point_cast<
            std::chrono::microseconds
            >(endTimepoint)
            .time_since_epoch()
            .count();

        uint32_t threadID =
            static_cast<uint32_t>(
                std::hash<std::thread::id>{}(
                    std::this_thread::get_id()
                    )
                );

        Instrumentor::Get().WriteProfile({
            m_Name,
            start,
            end,
            threadID
            });

        m_Stopped = true;
    }

}
