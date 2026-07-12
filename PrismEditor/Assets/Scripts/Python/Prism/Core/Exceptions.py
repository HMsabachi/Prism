class EngineException(Exception):
    pass


class MissingReferenceException(EngineException):
    def __init__(self, message="The native object has been destroyed or was never created."):
        super().__init__(message)


class EntityNotFoundException(EngineException):
    def __init__(self, message="The entity does not exist in the active scene."):
        super().__init__(message)

    @classmethod
    def by_id(cls, entity_id):
        return cls(f"Entity with ID {entity_id} not found in the active scene.")

    @classmethod
    def by_tag(cls, tag):
        return cls(f"Entity with tag '{tag}' not found in the active scene.")


class ComponentNotFoundException(EngineException):
    def __init__(self, message="The component was not found on the entity."):
        super().__init__(message)

    @classmethod
    def of_type(cls, component_type):
        return cls(f"Component of type '{component_type.__name__}' not found on the entity.")


class ResourceLoadException(EngineException):
    def __init__(self, message="Failed to load the resource."):
        super().__init__(message)

    @classmethod
    def at_path(cls, path):
        return cls(f"Failed to load resource at path: '{path}'")
