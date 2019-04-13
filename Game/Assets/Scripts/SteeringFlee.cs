using System.Collections;
using System;
using JellyBitEngine;

public class SteeringFlee : SteeringAbstract
{
    #region PUBLIC_VARIABLES
    public Agent agent = null;
    #endregion

    public Vector3 GetFlee()
    {
        if (agent == null)
            return Vector3.zero;

        Vector3 direction = (agent.transform.position - agent.Destination).normalized();
        direction *= agent.agentConfiguration.maxAcceleration;

        return direction;
    }
}