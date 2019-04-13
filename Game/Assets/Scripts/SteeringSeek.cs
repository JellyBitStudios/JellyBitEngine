using System.Collections;
using System;
using JellyBitEngine;

public class SteeringSeek : SteeringAbstract
{
    #region PUBLIC_VARIABLES
    public Agent agent = null;
    #endregion

    public Vector3 GetSeek()
    {
        if (agent == null)
            return Vector3.zero;

        Vector3 direction = (agent.Destination - agent.transform.position).normalized();
        direction *= agent.agentConfiguration.maxAcceleration;

        return direction;
    }
}