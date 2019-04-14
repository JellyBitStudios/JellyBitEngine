using System.Collections;
using System;
using JellyBitEngine;

public class SteeringAlignData : SteeringAbstract
{
    public float minAngle = 0.01f;
    public float slowAngle = 0.1f;
    public float timeToTarget = 0.1f;
}

public static class SteeringAlign
{
    public static float GetAlign(Agent agent)
    {
        if (agent == null)
            return 0.0f;

        float orientation = MathScript.Rad2Deg * (float)Math.Atan2(agent.transform.forward.x, agent.transform.forward.z);
        Vector3 direction = (agent.Destination - agent.transform.position).normalized();
        float targetOrientation = MathScript.Rad2Deg * (float)Math.Atan2(direction.x, direction.z);

        float diff = MathScript.DeltaAngle(orientation, targetOrientation); // wrap around PI
        float diffAbs = Math.Abs(diff);

        // Are we there (min radius)?
        if (diffAbs < agent.alignData.minAngle)
            // No acceleration
            return 0.0f;

        float targetRotation = 0.0f;
        // Are we outside the slow radius?
        if (diffAbs > agent.alignData.slowAngle)
            // Max rotation
            targetRotation = agent.agentData.maxAngularVelocity;
        else
            // Scaled rotation
            targetRotation = agent.agentData.maxAngularVelocity * diffAbs / agent.alignData.slowAngle;
    
        targetRotation *= diff / diffAbs;

        float angularAcceleration = targetRotation /*- orientation*/;
        angularAcceleration /= agent.alignData.timeToTarget;

        return Mathf.Clamp(angularAcceleration, -agent.agentData.maxAngularAcceleration, agent.agentData.maxAngularAcceleration);
    }

    public static void DrawGizmos(Agent agent)
    {

    }
}