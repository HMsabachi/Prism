using Prism;
using System;
using System.Text;
using System.Threading.Tasks;

namespace Example
{
    class Compute : Behaviour
    {

        public float SinkSpeed = 0;
        public ComputeShader CShader;
        private readonly float[] m_Input = new float[100];
        private ShaderStorageBuffer m_Buffer;
        private ShaderStorageBufferReadback? m_Request;
        private float m_Time;
        public void OnCreate()
        {
            CShader = ComputeShader.Create("Assets/Shaders/Test.ComputeShader");
            m_Buffer = ShaderStorageBuffer.Create((UInt32)(sizeof(float) * m_Input.Length), BufferUsage.Dynamic);
            int kernel = CShader.FindKernel("CSAdd");
            CShader.SetFloat(kernel, "u_Scale", 2.0f);
            CShader.SetVector3(kernel, "u_Offset", new Vector3(1.0f, 0.0f, 0.0f));
            CShader.SetInt(kernel, "u_Add", 1);
            CShader.SetBuffer(kernel, "u_Data", m_Buffer);
            m_Time = 0.0f;
        }

        public void OnUpdate()
        {
            m_Time += Time.DeltaTime;
            int kernel = CShader.FindKernel("CSAdd");
            CShader.Dispatch(kernel, (uint)((m_Input.Length + 63) / 64), 1, 1);

            if (m_Request == null && m_Time > 1.0f)
            {
                m_Request = m_Buffer.RequestReadback();
                m_Time = 0.0f;
            }

            if (m_Request != null && m_Request.IsDone)
            {
                float[] result = new float[m_Input.Length];
                m_Request.GetData(result);
                Log.Info($"Compute: output = [{string.Join(", ", result)}]");
                m_Request = null;
            }
        }

    }
}
