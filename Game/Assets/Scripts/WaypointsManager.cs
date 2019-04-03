using System.Collections;
using System.Collections.Generic;
using JellyBitEngine;

public class WaypointsManager : JellyScript
{
    #region PUBLIC_VARIABLES
    public Waypoint waypoint1 = null;
    public Waypoint waypoint2 = null;
    public Waypoint waypoint3 = null;
    public Waypoint waypoint4 = null;
    #endregion

    #region PRIVATE_VARIABLES
    private List<Waypoint> waypoints = new List<Waypoint>();
    #endregion

    public override void Start()
    {
        if (waypoint1 != null)
            waypoints.Add(waypoint1);
        if (waypoint2 != null)
            waypoints.Add(waypoint2);
        if (waypoint3 != null)
            waypoints.Add(waypoint3);
        if (waypoint4 != null)
            waypoints.Add(waypoint4);
    }

    public Waypoint GetClosestWaypoint(Vector3 position)
    {
        Debug.Log("Closest point requested");

        Waypoint closestWaypoint = null;
        float closestDistance = float.MaxValue;

        foreach (Waypoint waypoint in waypoints)
        {
            if (!waypoint.IsBlocked && !waypoint.isOccupied)
            {
                float waypointDistance = (float)(waypoint.transform.position - position).magnitude;
                if (waypointDistance < closestDistance)
                {
                    closestWaypoint = waypoint;
                    closestDistance = waypointDistance;
                }
            }
        }

        return closestWaypoint;
    }
}