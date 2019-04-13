using System.Collections;
using System;
using JellyBitEngine;

public class SteeringSeek : SteeringAbstract
{
    public Vector3 GetSeek(Agent agent)
    {
        if (agent == null)
            return Vector3.zero;

        Debug.Log("Agent position: " + agent.transform.position);
        Debug.Log("Agent destination: " + agent.Destination);
        Vector3 direction = agent.Destination - agent.transform.position;
        Debug.Log("Direction1: " + direction);
        direction.Normalize();
        Debug.Log("Direction: " + direction);
        direction *= agent.agentConfiguration.maxAcceleration;
        Debug.Log("Seek Direction: " + direction);

        return direction;
    }
}