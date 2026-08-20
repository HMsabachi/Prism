#include "prpch.h"
#include "Prism/Core/RenderThread.h"

#include "Prism/Renderer/Renderer.h"

#include <Windows.h>

namespace Prism
{
    struct RenderThreadData
    {
        CRITICAL_SECTION m_CriticalSection;
        CONDITION_VARIABLE m_ConditionVariable;
        RenderThread::State m_State = RenderThread::State::Idle;
    };

    static std::thread::id s_RenderThreadID;

    RenderThread::RenderThread(ThreadingPolicy policy)
        : m_RenderThread("Render Thread"), m_ThreadingPolicy(policy)
    {
        m_Data = new RenderThreadData();

        if (m_ThreadingPolicy == ThreadingPolicy::MultiThreaded)
        {
            InitializeCriticalSection(&m_Data->m_CriticalSection);
            InitializeConditionVariable(&m_Data->m_ConditionVariable);
        }
    }

    RenderThread::~RenderThread()
    {
        if (m_ThreadingPolicy == ThreadingPolicy::MultiThreaded)
            DeleteCriticalSection(&m_Data->m_CriticalSection);

        delete m_Data;
        m_Data = nullptr;
        s_RenderThreadID = std::thread::id();
    }

    void RenderThread::Run()
    {
        m_IsRunning = true;
        if (m_ThreadingPolicy == ThreadingPolicy::MultiThreaded)
            m_RenderThread.Dispatch(Renderer::RenderThreadFunc, this);

        s_RenderThreadID = m_RenderThread.GetID();
#if defined(PR_PLATFORM_WINDOWS)
#  if 1
        DWORD_PTR mask = 1ull << 0;
        SetThreadAffinityMask(GetCurrentThread(), mask);
        mask = 1ull << 2;
        HANDLE handle = reinterpret_cast<HANDLE>(m_RenderThread.GetNativeHandle());
        SetThreadAffinityMask(handle, mask);
#  endif
#endif
    }

    void RenderThread::Terminate()
    {
        m_IsRunning = false;
        Pump();

        if (m_ThreadingPolicy == ThreadingPolicy::MultiThreaded)
            m_RenderThread.Join();

        s_RenderThreadID = std::thread::id();
    }

    void RenderThread::Wait(State waitForState)
    {
        if (m_ThreadingPolicy == ThreadingPolicy::SingleThreaded)
            return;

        EnterCriticalSection(&m_Data->m_CriticalSection);
        while (m_Data->m_State != waitForState)
        {
            SleepConditionVariableCS(&m_Data->m_ConditionVariable, &m_Data->m_CriticalSection, INFINITE);
        }
        LeaveCriticalSection(&m_Data->m_CriticalSection);
    }

    void RenderThread::WaitAndSet(State waitForState, State setToState)
    {
        if (m_ThreadingPolicy == ThreadingPolicy::SingleThreaded)
            return;

        EnterCriticalSection(&m_Data->m_CriticalSection);
        while (m_Data->m_State != waitForState)
        {
            SleepConditionVariableCS(&m_Data->m_ConditionVariable, &m_Data->m_CriticalSection, INFINITE);
        }
        m_Data->m_State = setToState;
        WakeAllConditionVariable(&m_Data->m_ConditionVariable);
        LeaveCriticalSection(&m_Data->m_CriticalSection);
    }

    void RenderThread::Set(State setToState)
    {
        if (m_ThreadingPolicy == ThreadingPolicy::SingleThreaded)
            return;

        EnterCriticalSection(&m_Data->m_CriticalSection);
        m_Data->m_State = setToState;
        WakeAllConditionVariable(&m_Data->m_ConditionVariable);
        LeaveCriticalSection(&m_Data->m_CriticalSection);
    }

    void RenderThread::NextFrame()
    {
        m_AppThreadFrame++;
        if (m_ThreadingPolicy == ThreadingPolicy::MultiThreaded)
            Renderer::SwapQueues();
    }

    void RenderThread::BlockUntilRenderComplete()
    {
        if (m_ThreadingPolicy == ThreadingPolicy::SingleThreaded)
            return;

        Wait(State::Idle);
    }

    void RenderThread::Kick()
    {
        if (m_ThreadingPolicy == ThreadingPolicy::MultiThreaded)
        {
            Set(State::Kick);
        }
        else
        {
            Renderer::WaitAndRender();
        }
    }

    void RenderThread::Pump()
    {
        NextFrame();
        Kick();
        BlockUntilRenderComplete();
    }

    bool RenderThread::IsCurrentThreadRT()
    {
        return s_RenderThreadID == std::this_thread::get_id();
    }
}
