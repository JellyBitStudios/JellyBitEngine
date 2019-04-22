using System.Collections;
using System;
using JellyBitEngine;

public class SteeringRay
{
    public float length = 1.0f;
    public Vector3 direction = Vector3.forward;
}

public class SteeringObstacleAvoidanceData : SteeringAbstract
{
    public LayerMask mask = new LayerMask();
    public SteeringRay[] rays = new SteeringRay[3];
    public float avoidDistance = 1.0f; // Radius + avoidDistance
}

public static class SteeringObstacleAvoidance
{
    public static Vector3 GetObstacleAvoidance(Agent agent)
    {
        Vector3 outputAcceleration = Vector3.zero;

        if (agent == null)
            return outputAcceleration;

        foreach (SteeringRay steeringRay in agent.obstacleAvoidanceData.rays)
        {
            Ray ray = new Ray();
            ray.position = agent.transform.position;
            ray.direction = agent.transform.rotation * steeringRay.direction;
            if (agent.invertSight)
                ray.direction = Quaternion.Rotate(Vector3.up, 180.0f) * ray.direction;
            RaycastHit hitInfo;

            if (Physics.Raycast(ray, out hitInfo, steeringRay.length, agent.obstacleAvoidanceData.mask, SceneQueryFlags.Static | SceneQueryFlags.Dynamic))
            {
                Vector3 newAcceleration = hitInfo.point + hitInfo.normal * (agent.agentData.Radius + agent.obstacleAvoidanceData.avoidDistance);
                newAcceleration = new Vector3(newAcceleration.x, 0.0f, newAcceleration.z);
                if (newAcceleration.magnitude > outputAcceleration.magnitude)
                    outputAcceleration = newAcceleration;
            }
        }

        return outputAcceleration;
    }

    public static void DrawGizmos(Agent agent)
    {
        foreach (SteeringRay ray in agent.obstacleAvoidanceData.rays)
        {
            Vector3 direction = agent.transform.rotation * ray.direction * ray.length;
            if (agent.invertSight)
                direction = Quaternion.Rotate(Vector3.up, 180.0f) * direction;

            Debug.DrawLine(agent.transform.position, agent.transform.position + direction, Color.Blue);
        }
    }
}