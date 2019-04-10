using System.Collections;
using System;
using JellyBitEngine;

// https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Mathf.cs
// https://github.com/Sandruski/Tank-Game/blob/master/Behaviour%20Trees/Assets/Steering/SteeringAlign.cs

public class SteeringAlign : SteeringAbstract
{
    public float GetAlign(Agent agent)
    {
        if (agent == null)
            return 0.0f;

        float orientation = (float)Math.Atan2(agent.transform.forward.x, agent.transform.forward.z);
        Vector3 direction = (agent.destination - agent.transform.position).normalized();
        float targetOrientation = (float)Math.Atan2(direction.x, direction.z);

        float diff = DeltaAngle(orientation, targetOrientation);
        float diffAbs = Math.Abs(diff);
        // Are we there (min radius)?
        if (diffAbs < agent.agentConfiguration.alignMinAngle)
            // No acceleration
            return 0.0f;

        float targetRotation = 0.0f;
        // Are we outside the slow radius?
        if (diffAbs > agent.agentConfiguration.alignSlowAngle)
            // Max rotation
            targetRotation = agent.agentConfiguration.maxAngularVelocity;
        else
            // Scaled rotation
            targetRotation = diffAbs * agent.agentConfiguration.maxAngularVelocity / agent.agentConfiguration.alignSlowAngle;

        targetRotation *= NormalizedScalar(diff);

        float angularAcceleration = targetRotation - orientation;
        angularAcceleration /= agent.agentConfiguration.alignTimeToTarget;

        return Mathf.Clamp(angularAcceleration, -agent.agentConfiguration.maxAngularAcceleration, agent.agentConfiguration.maxAngularAcceleration);
    }

    float NormalizedScalar(float value)
    {
        return value / Math.Abs(value);
    }

    // ----------------------------------------------------------------------------------------------------

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