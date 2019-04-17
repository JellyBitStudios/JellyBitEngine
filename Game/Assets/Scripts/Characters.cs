using JellyBitEngine;
using System.Collections.Generic;

public class Character
{
    public int life = 100;
    public uint dmg = 0;

    public enum CharacterType
    {
        Alita,
        CyborgMelee,
        CyborgRanged
    }
    public CharacterType characterType;
}