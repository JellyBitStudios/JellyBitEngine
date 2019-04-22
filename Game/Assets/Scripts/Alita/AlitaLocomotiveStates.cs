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
    public virtual void ProcessRaycast(RaycastHit hit, bool leftClick) { } // Process raycast
    public virtual void ProcessEvent(Event latestEvent) { } // called when a Alita recieves a new Event
}

class AIdle : AState
{
    public override void OnStart()
    {
        Alita.Call.animator.UpdateAnimationBlendTime(0.1f);
        Alita.Call.animator.PlayAnimation("idle_alita_anim");
        Debug.Log("ANIMATION IDLE");

        Alita.Call.agent.isMovementStopped = true;
        Alita.Call.agent.isRotationStopped = false;
    }

    public override void ProcessInput(KeyCode code)
    {
        if (code == KeyCode.KEY_Q && Alita.Call.skillset.skQ.Use())
        {
            Debug.Log("Q HIT");
            Alita.Call.SwitchState(Alita.Call.StateSkill_Q);
        }
        else if (code == KeyCode.KEY_W && Alita.Call.skillset.skW.Use())
        {
            Debug.Log("W HIT");
            Alita.Call.StateSkill_W.SetDirection(Player.lastRaycastHit.point);
            Alita.Call.SwitchState(Alita.Call.StateSkill_W);
        }
    }

    public override void ProcessRaycast(RaycastHit hit, bool leftClick = true)
    {
        Alita.Call.agent.FinishFace();
        if (leftClick)
        {
            if (hit.gameObject.GetLayer() == "Terrain")
            {
                Vector3 point = new Vector3(hit.point.x, 0.0f, hit.point.z);
                Vector3 alita = new Vector3(Alita.Call.transform.position);
                alita.y = 0.0f;
                float diff = (point - alita).magnitude;
                if (diff > Alita_Entity.ConstMinRadiusToMove && Alita.Call.agent.SetDestination(hit.point))
                    Alita.Call.SwitchState(Alita.Call.StateWalking2Spot);
                else
                {
                    Vector3 dir = new Vector3();
                    dir = point - alita;
                    Alita.Call.agent.SetFace(dir);
                }
            }
            else if (hit.gameObject.GetLayer() == "Enemy")
            {
                // WARNING: SetDestination could return false but here we are targetting an enemy so...
                Alita.Call.currentTarget = hit.gameObject;
                Alita.Call.agent.SetDestination(hit.gameObject.transform.position);
                Alita.Call.SwitchState(Alita.Call.StateWalking2Enemy);
            }
        }
        else
        {
            if (Alita.Call.skillset.skDash.Use())
            {
                Alita.Call.StateDash.SetDirection(hit.point);
                Alita.Call.SwitchState(Alita.Call.StateDash);
            }
        }
    }
}

#region WALKING

class AWalking : AState
{
    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("anim_run_alita_fist");
        Alita.Call.agent.isMovementStopped = false;
        Alita.Call.agent.isRotationStopped = false;
    }

    public override void OnStop()
    {
        Alita.Call.agent.ClearPath();
        Alita.Call.agent.ClearMovementAndRotation();
    }

    public override void ProcessInput(KeyCode code)
    {
        if (code == KeyCode.KEY_Q && Alita.Call.skillset.skQ.Use())
        {
            Alita.Call.currentTarget = null;
            Alita.Call.SwitchState(Alita.Call.StateSkill_Q);
        }
        else if (code == KeyCode.KEY_W && Alita.Call.skillset.skW.Use())
        {
            Alita.Call.currentTarget = null;
            Alita.Call.StateSkill_W.SetDirection(Player.lastRaycastHit.point);
            Alita.Call.SwitchState(Alita.Call.StateSkill_W);
        }
    }
}

class AWalking2Spot : AWalking
{
    public override void OnExecute()
    {
        if (Alita.Call.agent.HasArrived)
            Alita.Call.SwitchState(Alita.Call.StateIdle);
    }

    public override void ProcessRaycast(RaycastHit hit, bool leftClick)
    {
        if (leftClick)
        {
            if (hit.gameObject.GetLayer() == "Terrain")
            {
                Vector3 point = new Vector3(hit.point.x, 0.0f, hit.point.z);
                Vector3 alita = new Vector3(Alita.Call.transform.position);
                alita.y = 0.0f;
                float diff = (point - alita).magnitude;
                if (diff > Alita_Entity.ConstMinRadiusToMove)
                    Alita.Call.agent.SetDestination(hit.point);
                else
                    Alita.Call.SwitchState(Alita.Call.StateIdle);
            }
            else if (hit.gameObject.GetLayer() == "Enemy")
            {
                Alita.Call.currentTarget = hit.gameObject;
                Alita.Call.agent.SetDestination(hit.gameObject.transform.position);
                Alita.Call.SwitchState(Alita.Call.StateWalking2Enemy, false, false);
            }
        }
        else
        {
            if (Alita.Call.skillset.skDash.Use())
            {
                Alita.Call.StateDash.SetDirection(hit.point);
                Alita.Call.SwitchState(Alita.Call.StateDash);
            }
        }
    }
}

class AWalking2Enemy : AWalking
{
    public override void OnExecute()
    {
        if (Alita.Call.currentTarget == null)
        {
            Debug.LogError("TARGET NULL at Walking2Enemy. Previous state"+ Alita.Call.lastState.ToString());
            return;
        }
        if ((Alita.Call.transform.position -
             Alita.Call.currentTarget.transform.position)
             .magnitude < Alita.Call.targetController.entity.DistanceToTarget)
        {
            Alita.Call.SwitchState(Alita.Call.StateAttacking);
        }
    }

    public override void ProcessRaycast(RaycastHit hit, bool leftClick)
    {
        if (leftClick)
        {
            if (hit.gameObject.GetLayer() == "Terrain")
            {
                Alita.Call.agent.SetDestination(hit.point);
                Alita.Call.SwitchState(Alita.Call.StateWalking2Spot, false, false);
                Alita.Call.currentTarget = null;
            }
            else if (hit.gameObject.GetLayer() == "Enemy")
            {
                if (Alita.Call.agent.SetDestination(hit.gameObject.transform.position))
                    Alita.Call.currentTarget = hit.gameObject;
            }
        }
        else
        {
            if (Alita.Call.skillset.skDash.Use())
            {
                Alita.Call.currentTarget = null;
                Alita.Call.StateDash.SetDirection(hit.point);
                Alita.Call.SwitchState(Alita.Call.StateDash);
            }
        }
    }
}

#endregion