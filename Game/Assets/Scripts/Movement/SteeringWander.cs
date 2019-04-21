using System.Collections;
using System;
using JellyBitEngine;

public class SteeringWanderData : SteeringAbstract
{
    public float radius = 1.0f;
    public float offset = 1.0f; // should be >= than the collider of the agent (probably a sphere collider)

    public float minTime = 0.5f;
    public float maxTime = 3.0f;

    public float time = 0.0f;
    public float timeToChange = 0.0f;
    public Vector3 dir = Vector3.zero;
    public Vector3 point = Vector3.zero;
}

public static class SteeringWander
{
    public static Vector3 GetWander(Agent agent)
    {
        if (agent == null)
            return Vector3.zero;

        if (agent.wanderData.time <= 0.0f)
        {
            // Change
            agent.wanderData.dir = new Vector3((float)MathScript.GetRandomDouble(-1.0f, 1.0f), 0.0f, (float)MathScript.GetRandomDouble(-1.0f, 1.0f));

            agent.wanderData.timeToChange = (float)MathScript.GetRandomDouble(agent.wanderData.minTime, agent.wanderData.maxTime);
            agent.wanderData.time = agent.wanderData.timeToChange;
        }

        agent.wanderData.time -= Time.deltaTime;

        Vector3 circlePos = agent.transform.position + agent.transform.forward * (agent.wanderData.offset + agent.agentData.Radius + agent.seekData.arriveMinDistance);
        agent.wanderData.point = circlePos + agent.wanderData.dir * agent.wanderData.radius;

        return SteeringSeek.GetSeek(agent.wanderData.point, agent);
    }

    public static void DrawGizmos(Agent agent)
    {
        Debug.DrawSphere(agent.wanderData.radius, Color.Red, agent.transform.position + agent.transform.forward * agent.wanderData.offset, Quaternion.identity, Vector3.one);
    }
}