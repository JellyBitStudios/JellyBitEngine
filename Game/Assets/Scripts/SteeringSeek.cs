using System.Collections;
using System;
using JellyBitEngine;

public class SteeringSeek : SteeringAbstract
{
    public Vector3 GetSeek(Agent agent)
    {
        if (agent == null)
            return Vector3.zero;

        Vector3 direction = (agent.Destination - agent.transform.position);
        Debug.Log("Direction: " + direction);
        direction.Normalize();
        direction *= agent.agentConfiguration.maxAcceleration;
        Debug.Log("Seek Direction: " + direction);

        return direction;
    }
}