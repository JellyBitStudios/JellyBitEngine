using System.Collections;
using System;
using JellyBitEngine;

public class SteeringSeekData : SteeringAbstract
{
    // Arrive
    public float arriveMinDistance = 1.0f;
}

public static class SteeringSeek
{
    public static Vector3 GetSeekPosition(Vector3 position, Agent agent)
    {
        if (agent == null)
            return Vector3.zero;

        Vector3 direction = position - agent.transform.position;
        if (direction.magnitude < agent.seekData.arriveMinDistance)
            return Vector3.zero;

        direction.Normalize();
        direction *= agent.agentData.maxAcceleration;

        return direction;
    }

    public static Vector3 GetSeekDirection(Vector3 direction, Agent agent)
    {
        if (agent == null)
            return Vector3.zero;

        direction.Normalize();
        direction *= agent.agentData.maxAcceleration;

        return direction;
    }

    public static void DrawGizmos(Agent agent)
    {

    }
}