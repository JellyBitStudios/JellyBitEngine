using System.Collections;
using System;
using JellyBitEngine;
using System.Collections.Generic;

public class AgentsManager : JellyScript
{
    #region PUBLIC_VARIABLES
    public static AgentsManager Call
    {
        get { return instance; }
    }

    public float Radius
    {
        get { return radius; }
    }
    #endregion

    #region PRIVATE_VARIABLES
    private static AgentsManager instance;
    private List<Agent> agents = new List<Agent>();

    private float radius = 0.0f;
    #endregion

    // ----------------------------------------------------------------------------------------------------

    public AgentsManager()
    {
        instance = this;
    }

    public bool AddAgent(Agent agent)
    {
        if (agent != null && !agents.Contains(agent))
        {
            agents.Add(agent);

            if (agent.agentData.Radius > radius)
                radius = agent.agentData.Radius;

            return true;
        }

        return false;
    }

    public bool RemoveAgent(Agent agent)
    {
        if (agent != null && agents.Contains(agent))
        {
            agents.Remove(agent);

            RecalculateRadius();

            return true;
        }

        return false;
    }

    // ----------------------------------------------------------------------------------------------------

    public void RecalculateRadius()
    {
        foreach (Agent agent in agents)
        {
            if (agent.agentData.Radius > radius)
                radius = agent.agentData.Radius;
        }
    }
}