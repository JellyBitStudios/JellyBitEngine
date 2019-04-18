using JellyBitEngine;

public class AState
{
    public virtual void OnAwake(Object controller) { } // Called at the beginning of the game
    public virtual void OnStart() { } // Called once at the beginning of the frame
    public virtual void OnExecute() { } // Called each frame
    public virtual void OnStop() { }  // Called once at the end of the state
    public virtual void OnPause() { } // Called when the game or character must be paused
    public virtual void OnResume() { } // Called to resume the game. OnPause previously called
    public virtual void ProcessInput(KeyCode code) { } // Process input
    public virtual void ProcessRaycast(RaycastHit hit) { } // Process raycast
    public virtual void ProcessEvent(Event latestEvent) { } // called when a Alita recieves a new Event
    public virtual void NewTarget(GameObject target) { }
}

class AIdle : AState
{
    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("idle_alita_anim");
    }

    public override void ProcessInput(KeyCode code)
    {
        if (code == KeyCode.KEY_Q)
            Alita.Call.SwitchState(Alita.Call.StateSkill_1);
        else if (code == KeyCode.KEY_SPACE)
            Alita.Call.SwitchState(Alita.Call.StateDash);
    }

    public override void ProcessRaycast(RaycastHit hit)
    {
        if (hit.gameObject.GetLayer() == "Terrain")
        {
            Alita.Call.agent.SetDestination(hit.point);
            Alita.Call.SwitchState(Alita.Call.StateWalking);
        }
    }
}

class AWalking : AState
{
    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("anim_run_alita_fist");

        if (Alita.Call.lastState == Alita.Call.StateSkill_1)
        {
            // resume Agent
        }
    }

    public override void OnExecute()
    {
        if (Alita.Call.agent.HasArrived)
            Alita.Call.SwitchState(Alita.Call.StateIdle);
    }

    public override void ProcessInput(KeyCode code)
    {
        if (code == KeyCode.KEY_Q)
            Alita.Call.SwitchState(Alita.Call.StateSkill_1);
        else if (code == KeyCode.KEY_SPACE)
            Alita.Call.SwitchState(Alita.Call.StateDash);
    }

    public override void ProcessRaycast(RaycastHit hit)
    {
        if (hit.gameObject.GetLayer() == "Terrain")
        {
            Alita.Call.agent.SetDestination(hit.point);
        }
    }
}

class AAttacking : AState
{
    enum Anim { first, second, third }
    Anim currentAnim;

    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("anim_basic_attack_alita_fist");
        currentAnim = Anim.first;
    }

    public override void OnExecute()
    {
        if (Alita.Call.animator.AnimationFinished())
        {
            if (currentAnim == Anim.first)
            {
                Alita.Call.animator.PlayAnimation("secondAttack_animation_alita");
                currentAnim = Anim.second;
            }
            else if (currentAnim == Anim.first)
            {
                Alita.Call.animator.PlayAnimation("thirdAttack_animation_alita");
                currentAnim = Anim.third;
            }
            else
            {
                Alita.Call.animator.PlayAnimation("anim_basic_attack_alita_fist");
                currentAnim = Anim.first;
            }
        }
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
        {
            Alita.Call.SwitchState(Alita.Call.StateIdle);
        }
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
        {
            if (Alita.Call.lastState == Alita.Call.StateWalking)
            {
                Alita.Call.SwitchState(Alita.Call.StateWalking);
                return;
            }            
            Alita.Call.SwitchState(Alita.Call.StateIdle);
        }
    }

    public override void OnStop()
    {
    }
}
