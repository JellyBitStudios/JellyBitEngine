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
    public float avoidDistance = 1.0f; // should be greater than the radius of the character
    public SteeringRay[] rays = null;
}

public static class SteeringObstacleAvoidance
{
    public static Vector3 GetObstacleAvoidance(Agent agent)
    {
        if (agent == null)
            return Vector3.zero;

        foreach (SteeringRay steeringRay in agent.obstacleAvoidanceData.rays)
        {
            RaycastHit hitInfo;
            Ray ray = new Ray();
            ray.position = agent.transform.position;
            ray.direction = steeringRay.direction;

            if (Physics.Raycast(ray, out hitInfo, steeringRay.length, agent.obstacleAvoidanceData.mask, SceneQueryFlags.Static))
            {
                Vector3 outputAcceleration = hitInfo.point + hitInfo.normal * agent.obstacleAvoidanceData.avoidDistance;
                outputAcceleration = new Vector3(outputAcceleration.x, 0.0f, outputAcceleration.z);
                return outputAcceleration;
            }
        }

        return Vector3.zero;
    }

    public static void DrawGizmos(Agent agent)
    {
        float[] color = { 0.0f, 0.0f, 1.0f, 1.0f };

        foreach (SteeringRay ray in agent.obstacleAvoidanceData.rays)
            Debug.DrawLine(agent.transform.position, agent.transform.position + agent.transform.rotation * ray.direction * ray.length, color);
    }
}