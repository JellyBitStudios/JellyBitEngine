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
    public Vector3 NextPosition
    {
        get
        {
            if (hasPath && index < path.Length - 1)
                return path[index];
            else
                return destination;
        }
    }
    public bool HasPath
    {
        get { return hasPath; }
    }
    #endregion

    #region PRIVATE_VARIABLES
    private Vector3 destination = Vector3.zero;
    private bool hasPath = false;
    private uint index = 0;
    private Vector3[] path = null;
    #endregion

    public bool GetPath(Vector3 origin, Vector3 destination)
    {
        hasPath = Navigation.GetPath(origin, destination, out path);
        this.destination = hasPath ? destination : origin;
        index = 0;
        return hasPath;
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

        float[] color = { 0.0f, 0.0f, 1.0f, 1.0f };

        for (uint i = 0; i < path.Length - 1; ++i)
            Debug.DrawLine(path[i], path[i + 1], color);
    }
}