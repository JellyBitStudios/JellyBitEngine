using System.Collections;
using System;
using JellyBitEngine;

public class SteeringSeparation : SteeringAbstract
{
    public Vector3 GetSeparation(Agent agent)
    {
        Vector3 outputAcceleration = Vector3.zero;

        if (agent == null)
            return outputAcceleration;

        OverlapHit[] hitInfo;
        if (Physics.OverlapSphere(agent.agentConfiguration.separationRadius, agent.transform.position, out hitInfo, agent.agentConfiguration.separationMask, SceneQueryFlags.Dynamic))
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
                if (distance < agent.agentConfiguration.separationThreshold)
                {
                    float strength = agent.agentConfiguration.maxAcceleration * (agent.agentConfiguration.separationThreshold - distance) / agent.agentConfiguration.separationThreshold;
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