using System.Collections;
using System;
using JellyBitEngine;

public class SteeringFleeData : SteeringAbstract
{

}

public static class SteeringFlee
{
    public static Vector3 GetFlee(Vector3 position, Agent agent)
    {
        if (agent == null)
            return Vector3.zero;

        Vector3 direction = (agent.transform.position - position).normalized();
        direction *= agent.agentData.maxAcceleration;

        direction = new Vector3(direction.x, 0.0f, direction.z);
        return direction;
    }

    public static void DrawGizmos(Agent agent)
    {

    }
}