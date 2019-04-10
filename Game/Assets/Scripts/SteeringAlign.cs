using System.Collections;
using System;
using JellyBitEngine;

public class SteeringAlign : SteeringAbstract
{
    public float minAngle = 0.01f;
    public float slowAngle = 0.1f;
    public float timeToTarget = 0.1f;

    public float GetAlign(Agent agent)
    {
        if (agent == null)
            return 0.0f;

        float orientation = Rad2Deg * (float)Math.Atan2(agent.transform.forward.x, agent.transform.forward.z);
        Vector3 direction = Rad2Deg * (agent.Destination - agent.transform.position).normalized();
        float targetOrientation = (float)Math.Atan2(direction.x, direction.z); // wrap around PI

        float diff = DeltaAngle(orientation, targetOrientation);
        float diffAbs = Math.Abs(diff);
        // Are we there (min radius)?
        if (diffAbs < minAngle)
            // No acceleration
            return 0.0f;

        float targetRotation = 0.0f;
        // Are we outside the slow radius?
        if (diffAbs > slowAngle)
            // Max rotation
            targetRotation = agent.agentConfiguration.maxAngularVelocity;
        else
            // Scaled rotation
            targetRotation = diffAbs * agent.agentConfiguration.maxAngularVelocity / slowAngle;

        targetRotation *= NormalizedScalar(diff);

        float angularAcceleration = targetRotation - orientation;
        angularAcceleration /= timeToTarget;

        return Mathf.Clamp(angularAcceleration, -agent.agentConfiguration.maxAngularAcceleration, agent.agentConfiguration.maxAngularAcceleration);
    }

    float NormalizedScalar(float value)
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