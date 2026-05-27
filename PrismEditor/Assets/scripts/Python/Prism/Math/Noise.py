import PrismNative as _Prism


class Noise:
    @staticmethod
    def PerlinNoise(x, y):
        return _Prism.Prism_Noise_PerlinNoise(x, y)
