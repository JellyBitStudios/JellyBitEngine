using System.Collections;
using System.Collections.Generic;
using JellyBitEngine;

public class WaypointsManager : JellyScript
{
    #region PUBLIC_VARIABLES
    public GameObject goWaypoint1 = null;
    public GameObject goWaypoint2 = null;
    public GameObject goWaypoint3 = null;
    public GameObject goWaypoint4 = null;
    #endregion

    #region PRIVATE_VARIABLES
    private List<Waypoint> waypoints = new List<Waypoint>();
    #endregion

    public override void Start()
    {
        if (goWaypoint1 != null)
            waypoints.Add(goWaypoint1.GetComponent<Waypoint>());
        if (goWaypoint2 != null)
            waypoints.Add(goWaypoint2.GetComponent<Waypoint>());
        if (goWaypoint3 != null)
            waypoints.Add(goWaypoint3.GetComponent<Waypoint>());
        if (goWaypoint4 != null)
            waypoints.Add(goWaypoint4.GetComponent<Waypoint>());
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
                    Debug.Log("Closest point updated");

                    closestWaypoint = waypoint;
                    closestDistance = waypointDistance;
                }
            }
        }

        return closestWaypoint;
    }
}