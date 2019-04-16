public class Character
{
    uint life = 0;
    uint dmg = 0;

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


    float dashDistance;
    public const float attackRotConst = 10.0f;
    public const float attackRadiusConst = 2.0f;
}