using System.Collections;
using System;
using JellyBitEngine;

public class SteeringSeparation : SteeringAbstract
{
    #region PUBLIC_VARIABLES
    public LayerMask mask = new LayerMask();
    public float radius = 1.0f;
    public float threshold = 1.0f;

    public Agent agent = null;
    #endregion

    public Vector3 GetSeparation()
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
                float distance = direction.magnitude;
                if (distance < threshold)
                {
                    Debug.Log("Threshold");
                    float strength = agent.agentConfiguration.maxAcceleration * (threshold - distance) / threshold;
                    direction.Normalize();
                    direction *= strength;

                    outputAcceleration += direction;
                }
            }

            if (outputAcceleration.magnitude > agent.agentConfiguration.maxAcceleration)
            {
                outputAcceleration.Normalize();
                outputAcceleration *= agent.agentConfiguration.maxAcceleration;
            }
        }

        return outputAcceleration;
    }

    public override void OnDrawGizmos()
    {
        Debug.Log("Gizmos position: " + agent.transform.position);
        Debug.Log("Gizmos radius: " + radius);
        float[] color = { 1.0f, 0.0f, 0.0f, 1.0f };
        Debug.DrawSphere(radius, color, agent.transform.position, Quaternion.identity, Vector3.one);
    }
}