using System.Collections;
using System;
using JellyBitEngine;

// https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Vector3.cs

public static class MathScript
{
    // A tiny floating point value (RO)
    public static readonly float Epsilon = float.Epsilon;

    // The infamous ''3.14159265358979...'' value (RO)
    public const float PI = (float)Math.PI;

    // Degrees-to-radians conversion constant (RO)
    public const float Deg2Rad = PI * 2.0f / 360.0f;

    // Radians-to-degrees conversion constant (RO)
    public const float Rad2Deg = 1.0f / Deg2Rad;

    // Calculates the shortest difference between two given angles
    public static float DeltaAngle(float current, float target)
    {
        float delta = Repeat((target - current), 360.0f);
        if (delta > 180.0f)
            delta -= 360.0f;
        return delta;
    }

    // Loops the value t, so that it is never larger than length and never smaller than 0
    public static float Repeat(float t, float length)
    {
        return Mathf.Clamp(t - Floor(t / length) * length, 0.0f, length);
    }

    // Returns the largest integer smaller to or equal to /f/
    public static float Floor(float f) { return (float)Math.Floor(f); }

    // Compares two floating point values if they are similar
    public static bool Approximately(float a, float b)
    {
        // If a or b is zero, compare that the other is less or equal to epsilon.
        // If neither a or b are 0, then find an epsilon that is good for
        // comparing numbers at the maximum magnitude of a and b.
        // Floating points have about 7 significant digits, so
        // 1.000001f can be represented while 1.0000001f is rounded to zero,
        // thus we could use an epsilon of 0.000001f for comparing values close to 1.
        // We multiply this epsilon by the biggest magnitude of a and b.
        return Math.Abs(b - a) < Math.Max(0.000001f * Math.Max(Math.Abs(a), Math.Abs(b)), Epsilon * 8);
    }
}