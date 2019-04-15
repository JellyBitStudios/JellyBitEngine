using System.Collections;
using System;
using JellyBitEngine;

public class SteeringFleeData : SteeringAbstract
{

}

public static class SteeringFlee
{
    public static Vector3 GetFlee(Agent agent)
    {
        if (agent == null)
            return Vector3.zero;

        Vector3 direction = (agent.transform.position - agent.NextPosition).normalized();
        direction *= agent.agentData.maxAcceleration;

        direction = new Vector3(direction.x, 0.0f, direction.z);
        return direction;
    }

    public static void DrawGizmos(Agent agent)
    {

    }
}