using System.Collections;
using System;
using JellyBitEngine;

public class SteeringSeekData : SteeringAbstract
{

}

public static class SteeringSeek
{
    public static Vector3 GetSeek(Agent agent)
    {
        if (agent == null)
            return Vector3.zero;

        Vector3 direction = (agent.NextPosition - agent.transform.position).normalized();
        direction *= agent.agentData.maxAcceleration;

        direction = new Vector3(direction.x, 0.0f, direction.z);
        return direction;
    }

    public static void DrawGizmos(Agent agent)
    {

    }
}