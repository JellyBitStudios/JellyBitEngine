using System.Collections;
using System;
using JellyBitEngine;

public class PathManager
{
    #region PUBLIC_VARIABLES
    public Vector3 Destination
    {
        get { return destination; }
    }
    public bool HasArrived
    {
        get { return path == null; }
    }
    #endregion

    #region PRIVATE_VARIABLES
    private Vector3 destination = Vector3.zero;
    private bool hasPath = false;
    private int index = 0;
    private Vector3[] path = null;
    #endregion

    public bool GetPath(Vector3 origin, Vector3 destination)
    {
        Vector3[] p = null; // ...
        hasPath = Navigation.GetPath(origin, destination, out p);
        hasPath = hasPath && p.Length > 1;

        // New path? Start following it!
        if (hasPath)
        {
            path = p; // ...
            this.destination = destination;
            index = 1;
        }
        // Old path? Keep following it!
        else if (path != null)
            hasPath = true;

        return hasPath;
    }

    public void ClearPath()
    {
        path = null;
        hasPath = false;
        destination = Vector3.zero;
        index = 0;
    }

    // ----------------------------------------------------------------------------------------------------

    public bool UpdateNextPosition()
    {
        if (hasPath)
        {
            if (index < path.Length - 1)
            {
                ++index;
                return true;
            }
            else
                // End of the path
                ClearPath();
        }

        return false;
    }

    public Vector3 GetNextPosition(Agent agent)
    {
        if (hasPath)
            return path[index];

        return agent.transform.position;
    }

    public float GetRemainingDistance(Agent agent)
    {
        if (hasPath)
        {
            Vector3 diff = path[index] - agent.transform.position;
            return diff.magnitude;
        }

        return 0.0f;
    }

    public void DrawGizmos()
    {
        if (!hasPath)
            return;

        for (uint i = 0; i < path.Length - 1; ++i)
            Debug.DrawLine(path[i], path[i + 1], Color.Red);
    }
}