using JellyBitEngine;
using System.Collections.Generic;

public class Character
{
    public Character()
    {
        currentLife = (int)maxLife;
    }

    // --------------------------------------------------

    public uint maxLife = 100;
    public int dmg = 20;

    public int currentLife = 0;
}