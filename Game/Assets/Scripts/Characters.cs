using JellyBitEngine;
using System.Collections.Generic;

public class Character
{
    public uint maxLife = 1000;
    public int dmg = 20;

    public int currentLife = 0;

    public Character()
    {
        currentLife = (int)maxLife;
    }
}