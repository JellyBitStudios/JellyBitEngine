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
    public bool HasPath
    {
        get { return hasPath; }
    }
    public bool IsLastPosition
    {
        get { return hasPath && index == path.Length - 1; }
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
        path = p; // ...

        hasPath = hasPath && path.Length > 1;

        if (hasPath)
        {
            this.destination = destination;
            index = 1;
        }
        else
        {
            path = null;
            this.destination = Vector3.zero;
            index = 0;
        }

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
        if (hasPath && index < path.Length - 1)
        {
            ++index;
            return true;
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