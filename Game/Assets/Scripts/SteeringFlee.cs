using System.Collections;
using System;
using JellyBitEngine;

public class SteeringFlee : SteeringAbstract
{
    public Vector3 GetFlee(Agent agent)
    {
        if (agent == null)
            return Vector3.zero;

        Vector3 direction = (agent.transform.position - agent.Destination).normalized();
        direction *= agent.agentConfiguration.maxAcceleration;
        return direction;
    }
}