using System;
using System.Text;
using System.Threading.Tasks;

namespace Prism
{
    public enum CursorMode
    {
        Normal = 0,
        Hidden = 1,
        Locked = 2
    }

    public class Input
    {

        public static bool IsKeyPressed(KeyCode keycode)
        {
            var type = typeof(Input);
            return IsKeyPressed_Native(keycode);
        }

        public static Vector2 GetMousePosition()
        {
            Vector2 position;
            unsafe
            {
                InternalCalls.Prism_Input_GetMousePosition(&position);
            }
            return position;
        }

        public static void SetCursorMode(CursorMode mode)
        {
            unsafe { InternalCalls.Prism_Input_SetCursorMode(mode); }
        }

        public static CursorMode GetCursorMode()
        {
            unsafe{ return InternalCalls.Prism_Input_GetCursorMode(); }
        }

        private unsafe static bool IsKeyPressed_Native(KeyCode keycode)
        {
            return InternalCalls.Prism_Input_IsKeyPressed(keycode);
        }

    }
}
