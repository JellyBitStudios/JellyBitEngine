using System.Collections;
using System;
using JellyBitEngine;

public class SteeringSeparationData : SteeringAbstract
{
    public LayerMask mask = new LayerMask();
    //public float radius = 1.0f;
    //public float threshold = 1.0f;
}

public static class SteeringSeparation
{
    public static Vector3 GetSeparation(Agent agent)
    {
        if (agent == null)
            return Vector3.zero;

        OverlapHit[] hitInfo;
        if (Physics.OverlapSphere(AgentsManager.Call.Radius, agent.transform.position, out hitInfo, agent.separationData.mask, SceneQueryFlags.Dynamic))
        {
            Vector3 outputAcceleration = Vector3.zero;

            foreach (OverlapHit hit in hitInfo)
            {
                if (hit == null)
                    continue;

                GameObject target = hit.gameObject;

                if (target == null)
                    continue;

                Agent targetAgent = target.GetComponent<Agent>();

                if (targetAgent == null)
                    continue;

                Vector3 direction = agent.transform.position - target.transform.position;

                float distance = direction.magnitude;
                if (distance < targetAgent.agentData.Radius)
                {
                    float strength = agent.agentData.maxAcceleration * (targetAgent.agentData.Radius - distance) / targetAgent.agentData.Radius;
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
        Debug.DrawSphere(AgentsManager.Call.Radius, Color.Red, agent.transform.position, Quaternion.identity, Vector3.one);
    }
}