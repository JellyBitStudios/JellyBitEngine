using System.Collections;
using System;
using JellyBitEngine;

public class SteeringCollisionAvoidanceData : SteeringAbstract
{
    public LayerMask mask = new LayerMask();
    public float radius = 1.0f;
    public float coneHalfAngle = 45.0f;
}

public static class SteeringCollisionAvoidance
{
    public static Vector3 GetCollisionAvoidance(Agent agent)
    {
        if (agent == null)
            return Vector3.zero;

        OverlapHit[] hitInfo;
        if (Physics.OverlapSphere(agent.collisionAvoidanceData.radius, agent.transform.position, out hitInfo, agent.collisionAvoidanceData.mask, SceneQueryFlags.Dynamic))
        {
            // 1. Find the target that's closest to collision
            float shortestTime = float.PositiveInfinity;
            GameObject firstTarget = null;
            float firstMinSeparation = 0.0f;
            float firstDistance = 0.0f;
            Vector3 firstRelativePos = Vector3.zero;
            Vector3 firstRelativeVel = Vector3.zero;

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

                Vector3 direction = (target.transform.position - agent.transform.position).normalized();
                float coneThreshold = (float)Math.Cos(MathScript.Deg2Rad * agent.collisionAvoidanceData.coneHalfAngle);
                float dot = MathScript.Dot(agent.transform.forward, direction);
                Debug.Log("Dot: " + dot);
                if (dot > coneThreshold)
                {
                    Vector3 relativePos = target.transform.position - agent.transform.position;
                    Vector3 relativeVel = targetAgent.velocity - agent.velocity;
                    float relativeSpeed = relativeVel.magnitude;
                    float timeToCollision = MathScript.Dot(relativePos, relativeVel) / (relativeSpeed * relativeSpeed);

                    // Is it going to be a collision?
                    float distance = relativePos.magnitude;
                    float minSeparation = distance - relativeSpeed * timeToCollision;
                    if (minSeparation > 2.0f * agent.collisionAvoidanceData.radius)
                        continue;

                    // Is it the shortest?
                    Debug.Log("timeToCollision: " + timeToCollision);
                    Debug.Log("shortestTime: " + timeToCollision);
                    if (timeToCollision > 0.0f && timeToCollision < shortestTime)
                    {
                        Debug.Log("Collisionnn");
                        shortestTime = timeToCollision;
                        firstTarget = target;
                        firstMinSeparation = minSeparation;
                        firstDistance = distance;
                        firstRelativePos = relativePos;
                        firstRelativeVel = relativeVel;
                    }
                }
            }

            // 2. Calculate the steering
            if (firstTarget == null)
                return Vector3.zero;

            Vector3 outputAcceleration = Vector3.zero;
            // Are we going to hit exactly or are we already colliding?
            if (firstMinSeparation <= 0.0f || firstDistance < 2.0f * agent.collisionAvoidanceData.radius)
                // Do the steering based on current position
                outputAcceleration = firstTarget.transform.position - agent.transform.position;
            else
                // Calculate the future relative position
                outputAcceleration = firstRelativePos + firstRelativeVel * shortestTime;

            // Avoid the target
            outputAcceleration.Normalize();
            outputAcceleration *= agent.agentData.maxAcceleration;

            Debug.Log("Avoidance");

            return outputAcceleration;
        }

        return Vector3.zero;
    }

    public static void DrawGizmos(Agent agent)
    {
        Debug.DrawSphere(agent.collisionAvoidanceData.radius, Color.Green, agent.transform.position, Quaternion.identity, Vector3.one);
        Debug.DrawLine(agent.transform.position, agent.transform.position + Quaternion.Rotate(Vector3.up, agent.collisionAvoidanceData.coneHalfAngle) * agent.transform.forward * 3.0f, Color.Green);
        Debug.DrawLine(agent.transform.position, agent.transform.position + Quaternion.Rotate(Vector3.up, -agent.collisionAvoidanceData.coneHalfAngle) * agent.transform.forward * 3.0f, Color.Green);
    }
}