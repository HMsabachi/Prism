using System;
using System.Text;
using System.Threading.Tasks;

namespace Prism
{
    public enum MouseButton
    {
        Button0 = 0,
        Button1 = 1,
        Button2 = 2,
        Button3 = 3,
        Button4 = 4,
        Button5 = 5,
        Left = Button0,
        Right = Button1,
        Middle = Button2
    }

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

        public static bool IsMouseButtonPressed(MouseButton button)
        {
            unsafe { return InternalCalls.Prism_Input_IsMouseButtonPressed(button); }
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
