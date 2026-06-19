using System;

namespace Prism
{
    public class EngineException : Exception
    {
        public EngineException() { }
        public EngineException(string message) : base(message) { }
        public EngineException(string message, Exception inner) : base(message, inner) { }
    }

    public class MissingReferenceException : EngineException
    {
        public MissingReferenceException() : base("The native object has been destroyed or was never created.")
        {
        }

        public MissingReferenceException(string message) : base(message) { }
        public MissingReferenceException(string message, Exception inner) : base(message, inner) { }
    }

    public class EntityNotFoundException : EngineException
    {
        public EntityNotFoundException() : base("The entity does not exist in the active scene.")
        {
        }

        public EntityNotFoundException(ulong entityID)
            : base($"Entity with ID {entityID} not found in the active scene.") { }

        public EntityNotFoundException(string tag)
            : base($"Entity with tag '{tag}' not found in the active scene.") { }

        public EntityNotFoundException(string message, Exception inner) : base(message, inner) { }
    }

    public class ComponentNotFoundException : EngineException
    {
        public ComponentNotFoundException() : base("The component was not found on the entity.")
        {
        }

        public ComponentNotFoundException(Type componentType)
            : base($"Component of type '{componentType.Name}' not found on the entity.") { }

        public ComponentNotFoundException(string message, Exception inner) : base(message, inner) { }
    }

    public class ResourceLoadException : EngineException
    {
        public ResourceLoadException() : base("Failed to load the resource.")
        {
        }

        public ResourceLoadException(string path)
            : base($"Failed to load resource at path: '{path}'") { }

        public ResourceLoadException(string message, Exception inner) : base(message, inner) { }
    }
}
