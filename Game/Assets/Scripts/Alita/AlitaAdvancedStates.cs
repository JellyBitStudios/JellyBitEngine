using JellyBitEngine;

class AAttacking : AState
{
    enum Anim { first, second, third }
    Anim currentAnim;

    private bool hit;
    private bool hasPetition;
    private bool firstPetition;
    private float timerUntilPetitionIsValid;
    private const float timeUntilValidPetitons = 0.3f;

    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("anim_basic_attack_alita_fist");
        Alita.Call.animator.SetAnimationLoop(false);
        currentAnim = Anim.first;
        hasPetition = false;
        firstPetition = true;
        hit = false;

        Alita.Call.agent.isRotationStopped = false;
        Alita.Call.agent.alignData.lookWhereYoureGoingData.isActive = false;
        Alita.Call.agent.alignData.faceData.isActive = true;
    }

    public override void OnExecute()
    {
        Alita.Call.agent.SetFace(Alita.Call.currentTarget);

        if (firstPetition)
            timerUntilPetitionIsValid += Time.deltaTime;

        if (!hit)
        {
            switch (currentAnim)
            {
                case Anim.first:
                    if (Alita.Call.animator.GetCurrentFrame() >= 16)
                    {
                        hit = true;
                        Alita.Call.cyborgMelee.CurrentLife -= Alita.Call.character.dmg;
                    }
                    break;
                case Anim.second:
                    if (Alita.Call.animator.GetCurrentFrame() >= 15)
                    {
                        hit = true;
                        Alita.Call.cyborgMelee.CurrentLife -= Alita.Call.character.dmg;
                    }
                    break;
                case Anim.third:
                    if (Alita.Call.animator.GetCurrentFrame() >= 11)
                    {
                        hit = true;
                        Alita.Call.cyborgMelee.CurrentLife -= Alita.Call.character.dmg;
                    }
                    break;
            }
        }

        if (Alita.Call.animator.AnimationFinished())
        {
            hit = false;
            if (hasPetition)
            {
                if (currentAnim == Anim.first)
                {
                    Alita.Call.animator.PlayAnimation("anim_hand_forward_alita_fist");
                    firstPetition = false;
                    Alita.Call.animator.SetAnimationLoop(false);
                    currentAnim = Anim.second;
                }
                else if (currentAnim == Anim.second)
                {
                    Alita.Call.animator.PlayAnimation("anim_kick_alita_fist");
                    Alita.Call.animator.SetAnimationLoop(false);
                    currentAnim = Anim.third;
                }
                else
                {
                    Alita.Call.animator.PlayAnimation("anim_basic_attack_alita_fist");
                    Alita.Call.animator.SetAnimationLoop(false);
                    currentAnim = Anim.first;
                }
                hasPetition = false;
            }
            else
            {
                Alita.Call.SwitchState(Alita.Call.StateIdle);
            }
        }
    }

    public override void ProcessRaycast(RaycastHit hit, bool leftClick)
    {
        if (leftClick)
        {
            if (hit.gameObject.GetLayer() == "Terrain")
            {
                Alita.Call.currentTarget = null;
                Alita.Call.agent.SetDestination(hit.point);
                Alita.Call.SwitchState(Alita.Call.StateWalking2Spot);
            }
            else if (hit.gameObject.GetLayer() == "Enemy")
            {
                GameObject targeted = hit.gameObject;
                if (targeted == Alita.Call.currentTarget)
                {
                    if (!firstPetition || (firstPetition && timerUntilPetitionIsValid >
                                                            timeUntilValidPetitons))
                        hasPetition = true;
                }
                else
                {
                    Alita.Call.currentTarget = hit.gameObject;
                    Alita.Call.agent.SetDestination(hit.gameObject.transform.position);
                    Alita.Call.SwitchState(Alita.Call.StateWalking2Enemy);
                }
            }
        }
    }

    public override void OnStop()
    {
        currentAnim = Anim.first;
        Alita.Call.agent.alignData.lookWhereYoureGoingData.isActive = true;
        Alita.Call.agent.alignData.faceData.isActive = false;
    }
}

class ADash : AState
{
    float accumulatedDistance = 0.0f;
    Vector3 dir = new Vector3();

    public void SetDirection(Vector3 position)
    {
        dir = (position - Alita.Call.transform.position).normalized();
    }

    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("alita_dash_anim");

        Alita.Call.agent.isRotationStopped = false;
        Alita.Call.agent.alignData.lookWhereYoureGoingData.isActive = false;
        Alita.Call.agent.alignData.faceData.isActive = true;
        //Alita.Call.agent.SetFace(dir);
    }

    public override void OnExecute()
    {
        Vector3 increase = Time.deltaTime * dir * AlitaCharacter.ConstDashStrength;
        Alita.Call.transform.position += increase;
        accumulatedDistance += increase.magnitude;
        if (accumulatedDistance >= AlitaCharacter.ConstMaxDistance)
            Alita.Call.SwitchState(Alita.Call.StateIdle);
    }

    public override void OnStop()
    {
        Alita.Call.agent.alignData.lookWhereYoureGoingData.isActive = true;
        Alita.Call.agent.alignData.faceData.isActive = false;
        Alita.Call.agent.ClearPath();
        accumulatedDistance = 0.0f;
    }
}

class ASkill1 : AState
{
    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("anim_special_attack_q_alita_fist");
    }

    public override void OnExecute()
    {
        if (Alita.Call.animator.AnimationFinished())
            Alita.Call.SwitchState(Alita.Call.StateIdle);
    }
}