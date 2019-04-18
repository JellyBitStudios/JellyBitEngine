using JellyBitEngine;
using System.Collections.Generic;

public class Character
{
    public int life = 1000;
    public uint dmg = 10;

    public enum CharacterType
    {
        Alita,
        CyborgMelee,
        CyborgRanged
    }
    public CharacterType characterType;
}