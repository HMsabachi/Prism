using System.Reflection;

namespace Prism
{

    public class ScriptInstance
    {
        public object Instance;
        public ScriptClass Class;
        public Entity Entity;

        public ScriptInstance(ScriptClass scriptClass, uint entityID, uint sceneID)
        {
            Class = scriptClass;
            Instance = scriptClass.Instantiate();
            Entity = new Entity();

            // 把Entity注入脚本
            var entityType = typeof(Entity);
            var entityIDField = entityType.GetProperty("EntityID");
            var sceneIDField = entityType.GetProperty("SceneID");
            entityIDField?.SetValue(Instance, entityID);
            sceneIDField?.SetValue(Instance, sceneID);
            if (entityIDField == null || sceneIDField == null)
                Log.Error("没有找到 EntityID 或 SceneID 字段");
        }

        public void InvokeOnCreate()
        {
            Class.OnCreate?.Invoke(Instance, null);
        }

        public void InvokeOnUpdate()
        {
            Class.OnUpdate?.Invoke(Instance, new object[] {});
        }
    }
}