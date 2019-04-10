using System.Collections;
using System;
using JellyBitEngine;

public class SteeringSeparation : SteeringAbstract
{
    public LayerMask mask = new LayerMask();
    public float radius = 1.0f;
    public float threshold = 1.0f;

    public Vector3 GetSeparation(Agent agent)
    {
        Vector3 outputAcceleration = Vector3.zero;

        if (agent == null)
            return outputAcceleration;

        OverlapHit[] hitInfo;
        if (Physics.OverlapSphere(radius, agent.transform.position, out hitInfo, mask, SceneQueryFlags.Dynamic))
        {
            foreach (OverlapHit hit in hitInfo)
            {
                if (hit == null)
                    continue;

                GameObject target = hit.gameObject;

                if (target == null)
                    continue;

                Vector3 direction = target.transform.position - agent.transform.position;
                float distance = (float)direction.magnitude;
                if (distance < threshold)
                {
                    float strength = agent.agentConfiguration.maxAcceleration * (threshold - distance) / threshold;
                    direction.Normalize();
                    direction *= strength;

                    outputAcceleration += direction;
                }              
            }

            if (outputAcceleration.magnitude > agent.agentConfiguration.maxAcceleration)
                outputAcceleration = outputAcceleration.normalized() * agent.agentConfiguration.maxAcceleration;
        }

        return outputAcceleration;
    }
}