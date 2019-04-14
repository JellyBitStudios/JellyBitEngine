using System.Collections;
using System;
using JellyBitEngine;

public class SteeringSeparationData : SteeringAbstract
{
    public LayerMask mask = new LayerMask();
    public float radius = 1.0f;
    public float threshold = 1.0f;
}

public static class SteeringSeparation
{
    public static Vector3 GetSeparation(Agent agent)
    {
        Vector3 outputAcceleration = Vector3.zero;

        if (agent == null)
            return outputAcceleration;

        OverlapHit[] hitInfo;
        if (Physics.OverlapSphere(agent.separationData.radius, agent.transform.position, out hitInfo, agent.separationData.mask, SceneQueryFlags.Dynamic))
        {
            foreach (OverlapHit hit in hitInfo)
            {
                if (hit == null)
                    continue;

                GameObject target = hit.gameObject;

                if (target == null)
                    continue;

                Vector3 direction = agent.transform.position - target.transform.position;
                float distance = direction.magnitude;
                if (distance < agent.separationData.threshold)
                {
                    float strength = agent.agentData.maxAcceleration * (agent.separationData.threshold - distance) / agent.separationData.threshold;
                    direction.Normalize();
                    direction *= strength;

                    outputAcceleration += direction;
                }
            }

            if (outputAcceleration.magnitude > agent.agentData.maxAcceleration)
            {
                outputAcceleration.Normalize();
                outputAcceleration *= agent.agentData.maxAcceleration;
            }
        }

        return outputAcceleration;
    }

    public static void DrawGizmos(Agent agent)
    {
        float[] color = { 1.0f, 0.0f, 0.0f, 1.0f };
        Debug.DrawSphere(agent.separationData.radius, color, agent.transform.position, Quaternion.identity, Vector3.one);
    }
}