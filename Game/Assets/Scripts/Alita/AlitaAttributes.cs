using JellyBitEngine;
using System.Collections.Generic;

public class Alita_Entity : Entity
{
    public uint lvl = 1;
    public const float expPerLvlModifier = 20.0f;
    private float _currentExp;
    public float currentExp
    {
        get //get method for returning value
        {
            return _currentExp;
        }
        set
        {
            if (value >= lvl * expPerLvlModifier)
            {
                lvl += 1;
                _currentExp = 0;
            }
            else
                _currentExp = value;
        }
    }

    public const float ConstMinRadiusToMove = 0.8f;

    // const basic attack
    public const float ConstHitRadius = 1.0f;
    public const uint ConstFirstHitDmg = 15;
    public const uint ConstSecondHitDmg = 20;
    public const uint ConstThirdHitDmg = 30;

    // const skill Q
    public const uint ConstSkillqDmg = 30;
    public const float ConstSkillqRadius = 2.0f;

    // const dash
    public const float ConstDashStrength = 8.0f;
    public const float ConstMaxDistance = 8.0f;
}

//---------------------------------- SKILLS  ------------------------------------//

public class Skill
{
    public bool isUnlocked;

    public float sk_totalCd;
    public float sk_currentCd;
    public float sk_normalizedCd = 1.0f;

    public Skill(float totalCD, bool locked)
    {
        isUnlocked = !locked;
        sk_totalCd = totalCD;
        sk_currentCd = sk_totalCd;
    }

    public bool IsAvailable
    {
        get
        {
            return isUnlocked && sk_normalizedCd >= 1.0f;
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
    // TODO G: skills will remain here for now. Them can be created outside and added to the skills via
    // skillset.add? how we can acces to each skill? think about it

    // skill->Dash
    public Skill skDash = new Skill(3.0f, false);

    // skill->Q
    public Skill skQ = new Skill(2.0f, false);

    // skill->W
    public Skill skW = new Skill(4.0f, false);

    List<Skill> skills = new List<Skill>();

    public Skillset()
    {
        skills.Add(skDash);
        skills.Add(skQ);
        skills.Add(skW);
    }

    public void UpdateTick()
    {
        foreach (Skill skill in skills)
        {
            if (skill.isUnlocked && skill.sk_currentCd < skill.sk_totalCd)
            {
                skill.sk_currentCd += Time.deltaTime;
                if (skill.sk_currentCd > skill.sk_totalCd)
                    skill.sk_currentCd = skill.sk_totalCd;
                skill.sk_normalizedCd = skill.sk_currentCd / skill.sk_totalCd;
            }
        }
    }
}