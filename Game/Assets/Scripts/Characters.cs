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

public class AlitaCharacter : Character
{
    public uint lvl = 1;
    public const float expPerLvlModifier = 20.0f;
    public float currentExp
    {
        get //get method for returning value
        {
            return currentExp;
        }
        set
        {
            if (value >= lvl * expPerLvlModifier)
            {
                lvl += 1;
                currentExp = 0;
            }
            else
                currentExp = value;
        }
    }

    public const float ConstDashStrength = 8.0f;
    public const float ConstMaxDistance = 4.0f;
}