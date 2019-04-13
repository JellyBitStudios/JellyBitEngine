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
        if (direction.magnitude > 0.0)
            direction.Normalize();
        direction *= agent.agentConfiguration.maxAcceleration;

        return direction;
    }
}