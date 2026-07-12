import random
from Prism.Math import Vector2
from Prism import Noise as PrismNoise


def InverseLerp(min_val, max_val, value):
    if abs(max_val - min_val) < 0.000001:
        return min_val
    return (value - min_val) / (max_val - min_val)


def GenerateNoiseMap(mapWidth, mapHeight, seed, scale, octaves, persistance, lacunarity, offset):
    noiseMap = [[0.0] * mapWidth for _ in range(mapHeight)]

    prng = random.Random(seed)
    octaveOffsets = []
    for i in range(octaves):
        offsetX = prng.randint(-100000, 100000) + offset.x
        offsetY = prng.randint(-100000, 100000) + offset.y
        octaveOffsets.append(Vector2(offsetX, offsetY))

    if scale <= 0:
        scale = 0.0001

    maxNoiseHeight = float('-inf')
    minNoiseHeight = float('inf')

    halfWidth = mapWidth / 2.0
    halfHeight = mapHeight / 2.0

    for y in range(mapHeight):
        for x in range(mapWidth):
            amplitude = 1.0
            frequency = 1.0
            noiseHeight = 0.0

            for i in range(octaves):
                sampleX = (x - halfWidth) / scale * frequency + octaveOffsets[i].x
                sampleY = (y - halfHeight) / scale * frequency + octaveOffsets[i].y

                perlinValue = PrismNoise.PerlinNoise(sampleX, sampleY)
                noiseHeight += perlinValue * amplitude

                amplitude *= persistance
                frequency *= lacunarity

            if noiseHeight > maxNoiseHeight:
                maxNoiseHeight = noiseHeight
            elif noiseHeight < minNoiseHeight:
                minNoiseHeight = noiseHeight
            noiseMap[x][y] = noiseHeight

    for y in range(mapHeight):
        for x in range(mapWidth):
            noiseMap[x][y] = InverseLerp(minNoiseHeight, maxNoiseHeight, noiseMap[x][y])

    return noiseMap
