using JellyBitEngine;
using System.Collections.Generic;

public class Entity
{
    public Entity()
    {
        currentLife = (int)maxLife;
    }

    // --------------------------------------------------
    public string name = "Entity";

    public uint maxLife = 100;
    public int dmg = 20;

    public int currentLife = 0;
}

// Every entity that Alita can select as current target should inherit from this (enemies, chests, etc)
public class NPC_Entity : Entity
{
    // Alita uses this var as distance hit. Every entity, in function of its weight, should redefine it.
    public float DistanceToTarget = 2.0f;
}