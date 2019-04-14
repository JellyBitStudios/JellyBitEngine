using System.Collections;
using System;
using JellyBitEngine;

// https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Vector3.cs

public static class MathScript
{
    public static float NormalizedScalar(float value)
    {
        return value / Math.Abs(value);
    }

    // ----------------------------------------------------------------------------------------------------

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
}