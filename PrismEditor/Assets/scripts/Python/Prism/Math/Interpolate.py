class Interpolate:
    @staticmethod
    def Linear(p1, p2, t):
        if t < 0.0:
            return p1
        if t > 1.0:
            return p2
        return p1 + (p2 - p1) * t

    @staticmethod
    def EaseIn(a, b, t):
        return Interpolate.Linear(a, b, Interpolate._EaseIn(t))

    @staticmethod
    def EaseOut(a, b, t):
        return Interpolate.Linear(a, b, Interpolate._EaseOut(t))

    @staticmethod
    def EaseInOut(a, b, t):
        return Interpolate.Linear(a, b, Interpolate._EaseInOut(t))

    @staticmethod
    def _Invert(x):
        return 1.0 - x

    @staticmethod
    def _Square(x):
        return x * x

    @staticmethod
    def _EaseIn(t):
        return Interpolate._Square(t)

    @staticmethod
    def _EaseOut(t):
        return Interpolate._Invert(Interpolate._Square(Interpolate._Invert(t)))

    @staticmethod
    def _EaseInOut(t):
        return Interpolate.Linear(Interpolate._EaseIn(t), Interpolate._EaseOut(t), t)
