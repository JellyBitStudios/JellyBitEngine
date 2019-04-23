﻿using JellyBitEngine;
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
    public int currentLife = 0;

    public enum Action { selected, hit, thirdHit, skillQ, skillW }
}

// Every entity that Alita can select as current target should inherit from this (enemies, chests, etc)
public class NPC_Entity : Entity
{
    // Alita uses this var as distance hit. Every entity, in function of its weight, should redefine it.
    public float distanceToTarget = 2.0f;
    public uint dmg = 10;
    public bool isBoss = false;
}