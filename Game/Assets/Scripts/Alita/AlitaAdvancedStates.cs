using JellyBitEngine;
using System;

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
        currentAnim = Anim.first;
        hasPetition = false;
        firstPetition = true;
        hit = false;

        Alita.Call.agent.isMovementStopped = true;
        Alita.Call.agent.isRotationStopped = false;
        Alita.Call.agent.alignData.lookWhereYoureGoingData.isActive = false;
        Alita.Call.agent.alignData.faceData.isActive = true;
        Alita.Call.agent.SetFace(Alita.Call.currentTarget);
    }

    public override void OnExecute()
    {
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
                        Alita.Call.targetController.Actuate(Alita_Entity.ConstFirstHitDmg,
                                                            Entity.Action.hit);
                    }
                    break;
                case Anim.second:
                    if (Alita.Call.animator.GetCurrentFrame() >= 15)
                    {
                        hit = true;
                        Alita.Call.targetController.Actuate(Alita_Entity.ConstSecondHitDmg,
                                                            Entity.Action.hit);
                    }
                    break;
                case Anim.third:
                    if (Alita.Call.animator.GetCurrentFrame() >= 11)
                    {
                        hit = true;
                        Alita.Call.targetController.Actuate(Alita_Entity.ConstThirdHitDmg,
                                                            Entity.Action.thirdHit);
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
                    currentAnim = Anim.second;
                }
                else if (currentAnim == Anim.second)
                {
                    Alita.Call.animator.PlayAnimation("anim_kick_alita_fist");
                    currentAnim = Anim.third;
                }
                else
                {
                    Alita.Call.animator.PlayAnimation("anim_basic_attack_alita_fist");
                    currentAnim = Anim.first;
                }
                hasPetition = false;
            }
            else
            {
                Alita.Call.currentTarget = null;
                Alita.Call.SwitchState(Alita.Call.StateIdle);
            }
        }
    }

    public override void ProcessInput(KeyCode code)
    {
        // Should process input?
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
        Alita.Call.agent.isMovementStopped = false;
        Alita.Call.agent.alignData.lookWhereYoureGoingData.isActive = true;
        Alita.Call.agent.alignData.faceData.isActive = false;
    }
}

class ADash : AState
{
    float accumulatedDistance = 0.0f;
    float maxVelocity = 0.0f;
    Vector3 dir = new Vector3();

    public void SetDirection(Vector3 position)
    {
        dir = (position - Alita.Call.transform.position).normalized();
    }

    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("alita_dash_anim");

        Alita.Call.agent.Stop();

        //Points to Bezier curve 
        Vector3 p0 = new Vector3(Alita_Entity.ConstDashStrength, Alita_Entity.ConstDashStrength, Alita_Entity.ConstDashStrength);
        Vector3 p1 = new Vector3(Alita_Entity.ConstDashStrength / 2, Alita_Entity.ConstDashStrength / 2, Alita_Entity.ConstDashStrength / 2);
        Vector3 p2 = new Vector3(Alita_Entity.ConstDashStrength / 3, Alita_Entity.ConstDashStrength / 3, Alita_Entity.ConstDashStrength / 3);
        Vector3 p3 = new Vector3(2.0f, 2.0f, 2.0f);

        MathScript.BezierCurve.CreateBezierCurve(p0, p1, p2, p3);

        float targetOrientation = MathScript.Rad2Deg * (float)Math.Atan2(dir.x, dir.z);
        Quaternion quat = Quaternion.Rotate(Vector3.up, targetOrientation);
        Alita.Call.transform.rotation = quat;

        // Agent
        maxVelocity = Alita.Call.agent.agentData.maxVelocity;

        Alita.Call.agent.isMovementStopped = false;
        Alita.Call.agent.isRotationStopped = true;
        Alita.Call.agent.ActivateAvoidance();

        Alita.Call.agent.direction = Alita.Call.transform.forward;
        Alita.Call.agent.useDirection = true;
    }

    public override void OnExecute()
    {
        float time_bezier = accumulatedDistance / Alita_Entity.ConstMaxDistance;
        Vector3 bezier = MathScript.BezierCurve.GetPointOnBezierCurve(time_bezier);

        Alita.Call.agent.agentData.maxVelocity = bezier.magnitude;

        Vector3 increase = Time.fixedDeltaTime * dir * bezier.magnitude;
        accumulatedDistance += increase.magnitude;
        if (accumulatedDistance >= Alita_Entity.ConstMaxDistance)
            Alita.Call.SwitchState(Alita.Call.StateIdle);
    }

    public override void OnStop()
    {
        // Agent
        Alita.Call.agent.agentData.maxVelocity = maxVelocity;

        Alita.Call.agent.Resume();
        Alita.Call.agent.useDirection = false;

        accumulatedDistance = 0.0f;
    }
}

class ASkillQ : AState
{
    bool hit;

    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("anim_special_attack_q_alita_fist");
        Alita.Call.animator.SetAnimationLoop(false);
        hit = false;
    }

    public override void OnExecute()
    {
        if (!hit && Alita.Call.animator.GetCurrentFrame() >= 27)
        {
            Debug.Log("DAMAAAAAAAAAAGEEEEE");
            // overlap sphere
            // SEND DECAL
            OverlapHit[] hitInfo;
            Physics.OverlapSphere(Alita_Entity.ConstSkillqRadius, Alita.Call.transform.position, out hitInfo, LayerMask.GetMask("Enemy"), SceneQueryFlags.Static | SceneQueryFlags.Dynamic);

            if (hitInfo != null)
            {
                foreach (OverlapHit goHit in hitInfo)
                    goHit.gameObject.GetComponent<Controller>().Actuate(Alita_Entity.ConstSkillqDmg, Entity.Action.skillQ);
            }
            hit = true;
        }

        if (Alita.Call.animator.AnimationFinished())
            Alita.Call.SwitchState(Alita.Call.StateIdle);
    }
}

class ASkillW : AState
{
    bool hit;
    Vector3 dir = new Vector3();

    public void SetDirection(Vector3 position)
    {
        dir = (position - Alita.Call.transform.position).normalized();
    }

    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("alita_dash_anim");
        hit = false;
        Alita.Call.agent.Stop();

        float targetOrientation = MathScript.Rad2Deg * (float)Math.Atan2(dir.x, dir.z);
        Quaternion quat = Quaternion.Rotate(Vector3.up, targetOrientation);
        Alita.Call.transform.rotation = quat;
    }

    public override void OnExecute()
    {
        if (!hit && Alita.Call.animator.GetCurrentFrame() >= 27)
        {

            hit = true;
        }

        if (Alita.Call.animator.AnimationFinished())
            Alita.Call.SwitchState(Alita.Call.StateIdle);
    }

    public override void OnStop()
    {
        Alita.Call.agent.Resume();
    }
}
