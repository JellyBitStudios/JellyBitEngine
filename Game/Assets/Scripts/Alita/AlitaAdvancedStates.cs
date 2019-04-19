using JellyBitEngine;

class AAttacking : AState
{
    enum Anim { first, second, third }
    Anim currentAnim;

    private bool hasPetition;

    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("anim_basic_attack_alita_fist");
        currentAnim = Anim.first;
        hasPetition = false;

        Alita.Call.agent.isRotationStopped = false;
        Alita.Call.agent.alignData.lookWhereYoureGoingData.isActive = false;
        Alita.Call.agent.alignData.faceData.isActive = true;
    }

    public override void OnExecute()
    {
        Alita.Call.agent.SetFace(Alita.Call.currentTarget);

        if (Alita.Call.animator.AnimationFinished())
        {
            if (hasPetition)
            {
                if (currentAnim == Anim.first)
                {
                    Alita.Call.animator.PlayAnimation("anim_hand_forward_alita_fist");
                    currentAnim = Anim.second;
                    Alita.Call.cyborgMelee.CurrentLife -= Alita.Call.character.dmg;
                }
                else if (currentAnim == Anim.second)
                {
                    Alita.Call.animator.PlayAnimation("anim_kick_alita_fist");
                    currentAnim = Anim.third;
                    Alita.Call.cyborgMelee.CurrentLife -= Alita.Call.character.dmg;
                }
                else
                {
                    Alita.Call.animator.PlayAnimation("anim_basic_attack_alita_fist");
                    currentAnim = Anim.first;
                    Alita.Call.cyborgMelee.CurrentLife -= Alita.Call.character.dmg;
                }
                hasPetition = false;
            }
            else
            {
                Alita.Call.SwitchState(Alita.Call.StateIdle);
            }
        }
    }

    public override void ProcessRaycast(RaycastHit hit)
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
                hasPetition = true;
            else
            {
                Alita.Call.currentTarget = hit.gameObject;
                Alita.Call.agent.SetDestination(hit.gameObject.transform.position);
                Alita.Call.SwitchState(Alita.Call.StateWalking2Enemy);
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

    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("alita_dash_anim");
        dir = Alita.Call.gameObject.transform.forward;

        Alita.Call.agent.Stop();
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
        Alita.Call.agent.Reset();
        Alita.Call.agent.ClearPath();
        Alita.Call.agent.ClearMovementAndRotation();
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
