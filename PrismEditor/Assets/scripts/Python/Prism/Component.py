class Component:
    """Base class for all components. Holds a reference to the owning Entity."""

    def __init__(self):
        self.entity = None
