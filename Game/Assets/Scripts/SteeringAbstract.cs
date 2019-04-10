using System.Collections;
using System;
using JellyBitEngine;

static public class SteeringConfiguration
{
    public const uint maxPriorities = 3;
}

public class SteeringAbstract : JellyScript
{
    public uint Priority
    {
        get { return priority; }
        set
        {
            if (value >= 0 && value <= SteeringConfiguration.maxPriorities)
                priority = value;
            else
                priority = SteeringConfiguration.maxPriorities;
        }
    }
    private uint priority = 0;
}