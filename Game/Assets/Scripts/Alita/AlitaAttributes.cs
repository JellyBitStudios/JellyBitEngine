using JellyBitEngine;
using System.Collections.Generic;

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

    public const float ConstHitRadius = 1.0f;
    public const float ConstDashStrength = 8.0f;
    public const float ConstMaxDistance = 4.0f;
}

//---------------------------------- SKILLS  ------------------------------------//

public class Skill
{
    public float sk_totalCd = 0.0f;
    public float sk_currentCd = 0.0f;
    public float sk_normalizedCd = 0.0f;

    public bool IsAvailable
    {
        get
        {
            return sk_normalizedCd >= 1.0f;
        }
    }

    public bool Use()
    {
        if (IsAvailable)
        {
            sk_currentCd = 0.0f;
            return true;
        }
        return false;
    }
}

public class Skillset
{
    // skill->Dash
    public Skill skDash = new Skill();

    // skill->Q
    public Skill skQ = new Skill();

    List<Skill> skills = new List<Skill>();

    public Skillset()
    {
        skills.Add(skDash);
        skills.Add(skQ);
    }

    public void UpdateTick()
    {
        foreach (Skill skill in skills)
        {
            if (skill.sk_currentCd < skill.sk_totalCd)
            {
                skill.sk_currentCd += Time.deltaTime;
                if (skill.sk_currentCd > skill.sk_totalCd)
                    skill.sk_currentCd = skill.sk_totalCd;

                skill.sk_normalizedCd = skill.sk_totalCd / skill.sk_currentCd;
            }
        }
    }
}