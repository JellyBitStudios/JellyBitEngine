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
        if (agent == null)
            return Vector3.zero;

        OverlapHit[] hitInfo;
        if (Physics.OverlapSphere(agent.separationData.radius, agent.transform.position, out hitInfo, agent.separationData.mask, SceneQueryFlags.Dynamic))
        {
            Vector3 outputAcceleration = Vector3.zero;

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

            outputAcceleration = new Vector3(outputAcceleration.x, 0.0f, outputAcceleration.z);
            return outputAcceleration;
        }

        return Vector3.zero;
    }

    public static void DrawGizmos(Agent agent)
    {
        Debug.DrawSphere(agent.separationData.radius, Color.Red, agent.transform.position, Quaternion.identity, Vector3.one);
        Debug.DrawSphere(agent.separationData.threshold, Color.Red, agent.transform.position, Quaternion.identity, Vector3.one);
    }
}