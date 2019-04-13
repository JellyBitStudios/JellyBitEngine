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
            Debug.Log("Overlap");
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
                    Debug.Log("Threshold");
                    float strength = agent.agentConfiguration.maxAcceleration * (threshold - distance) / threshold;
                    if (direction.magnitude > 0.0)
                        direction.Normalize();
                    direction *= strength;

                    outputAcceleration += direction;
                }

                Debug.Log("Foreach");
            }

            if (outputAcceleration.magnitude > agent.agentConfiguration.maxAcceleration)
            {
                if (outputAcceleration.magnitude > 0.0)
                    outputAcceleration.Normalize();
                Debug.Log("Multiply");
                outputAcceleration *= agent.agentConfiguration.maxAcceleration;
            }
        }

        return outputAcceleration;
    }

    /*
    public override void OnDrawGizmos()
    {
        float[] color = { 1.0f, 0.0f, 0.0f, 1.0f };
        Debug.DrawSphere(radius, color, transform.position, Quaternion.identity, Vector3.one);
    }*/
}