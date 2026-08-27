#include "prpch.h"
#include "Prism/Core/Thread.h"

#include <Windows.h>

namespace Prism
{
    Thread::Thread(const std::string& name)
        : m_Name(name)
    {
    }

    Thread::~Thread()
    {
        Join();
    }

    void Thread::SetName(const std::string& name)
    {
        HANDLE threadHandle = m_Thread.native_handle();
        std::wstring wName(name.begin(), name.end());
        SetThreadDescription(threadHandle, wName.c_str());
    }

    void Thread::Join()
    {
        if (m_Thread.joinable())
            m_Thread.join();
    }

    ThreadSignal::ThreadSignal(const std::string& name, bool manualReset)
    {
        std::wstring str(name.begin(), name.end());
        m_SignalHandle = CreateEvent(NULL, (BOOL)manualReset, FALSE, str.c_str());
    }

    void ThreadSignal::Wait()
    {
        WaitForSingleObject(m_SignalHandle, INFINITE);
    }

    void ThreadSignal::Signal()
    {
        SetEvent(m_SignalHandle);
    }

    void ThreadSignal::Reset()
    {
        ResetEvent(m_SignalHandle);
    }

    std::thread::id Thread::GetID() const
    {
        return m_Thread.get_id();
    }

    std::thread::native_handle_type Thread::GetNativeHandle()
    {
        return m_Thread.native_handle();
    }

}
