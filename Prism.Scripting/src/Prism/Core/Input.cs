using System;
using System.Text;
using System.Threading.Tasks;

namespace Prism
{
    public class Input
    {

        public static bool IsKeyPressed(KeyCode keycode)
        {
            var type = typeof(Input);
            return IsKeyPressed_Native(keycode);
        }
        private unsafe static bool IsKeyPressed_Native(KeyCode keycode)
        {
            return InternalCalls.Prism_Input_IsKeyPressed(keycode);
        }

    }
}
