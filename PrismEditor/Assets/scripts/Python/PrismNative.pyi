# PrismNative 原生模块类型存根
# 此模块由 C++ NativeModule 在运行时动态创建

def Prism_Log_LogMessage(level: int, message: str) -> None:
    """记录日志
    Args:
        level: LogLevel 枚举值 (Trace=1, Debug=2, Info=4, Warn=8, Error=16, Critical=32)
        message: 日志消息
    """

def Prism_Time_GetDeltaTime() -> float:
    """获取上一帧到当前帧的时间差（秒）"""

def Prism_Time_GetTime() -> float:
    """获取自引擎启动以来的时间（秒）"""

def Prism_Input_IsKeyPressed(key: int) -> bool:
    """检查按键是否被按下
    Args:
        key: KeyCode 键码值
    """

def Prism_Entity_GetTransform(entityID: int) -> tuple[float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float]:
    """获取实体的变换矩阵
    Args:
        entityID: 实体 ID
    Returns:
        16 个 float 组成的 tuple（列主序矩阵）
    """

def Prism_Entity_SetTransform(entityID: int, mat: tuple[float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float]) -> None:
    """设置实体的变换矩阵
    Args:
        entityID: 实体 ID
        mat: 16 个 float 组成的 tuple（列主序矩阵）
    """

def Prism_Entity_CreateComponent(entityID: int, typeName: str) -> None:
    """为实体创建组件
    Args:
        entityID: 实体 ID
        typeName: 组件类型名称 (如 "TransformComponent", "MeshComponent")
    """

def Prism_Entity_HasComponent(entityID: int, typeName: str) -> bool:
    """检查实体是否拥有指定类型的组件
    Args:
        entityID: 实体 ID
        typeName: 组件类型名称
    """

def Prism_TransformComponent_GetPosition(entityID: int) -> tuple[float, float, float]:
    """获取实体的位置
    Args:
        entityID: 实体 ID
    Returns:
        (x, y, z) 位置坐标
    """

def Prism_TransformComponent_SetPosition(entityID: int, pos: tuple[float, float, float]) -> None:
    """设置实体的位置
    Args:
        entityID: 实体 ID
        pos: (x, y, z) 位置坐标
    """
