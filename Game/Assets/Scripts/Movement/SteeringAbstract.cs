using System.Collections;
using System;
using JellyBitEngine;

static public class SteeringData
{
    public const uint maxPriorities = 3;
}

public class SteeringAbstract
{
    #region PUBLIC_VARIABLES
    public bool isActive = true;
    public bool hasOutput = false;
    public uint Priority
    {
        get { return priority; }
        set
        {
            if (value >= 0 && value <= SteeringData.maxPriorities)
                priority = value;
            else
                priority = SteeringData.maxPriorities;
        }
    }
    #endregion

    #region PRIVATE_VARIABLES
    private uint priority = 0;
    #endregion
}