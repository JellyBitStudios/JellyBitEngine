using System.Collections;
using System;
using JellyBitEngine;

public class SteeringSeekData : SteeringAbstract
{
    // Arrive
    public float arriveMinDistance = 0.6f;
}

public static class SteeringSeek
{
    public static Vector3 GetSeek(Agent agent)
    {
        if (agent == null)
            return Vector3.zero;

        Vector3 direction = agent.NextPosition - agent.transform.position;
        if (direction.magnitude < agent.seekData.arriveMinDistance)
            return Vector3.zero;

        direction.Normalize();
        direction *= agent.agentData.maxAcceleration;

        direction = new Vector3(direction.x, 0.0f, direction.z);
        return direction;
    }

    public static void DrawGizmos(Agent agent)
    {

    }
}