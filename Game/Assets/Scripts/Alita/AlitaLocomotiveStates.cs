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
}

class AIdle : AState
{
    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("idle_alita_anim");
    }

    public override void ProcessInput(KeyCode code)
    {
        //if (code == KeyCode.KEY_Q && Alita.Call.skillset.skQ.Use())
        //    Alita.Call.SwitchState(Alita.Call.StateSkill_1);
        //else if (code == KeyCode.KEY_SPACE && Alita.Call.skillset.skDash.Use())
        //    Alita.Call.SwitchState(Alita.Call.StateDash);
    }

    public override void ProcessRaycast(RaycastHit hit)
    {
        if (hit.gameObject.GetLayer() == "Terrain")
        {
            if (Alita.Call.agent.SetDestination(hit.point))
                Alita.Call.SwitchState(Alita.Call.StateWalking2Spot);
        }
        else if (hit.gameObject.GetLayer() == "Enemy")
        {
            // WARNING: SetDestination could return false but here we are targetting an enemy so...
            Alita.Call.currentTarget = hit.gameObject;
            Alita.Call.agent.SetDestination(hit.gameObject.transform.position);
            Alita.Call.SwitchState(Alita.Call.StateWalking2Enemy);
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
        Alita.Call.agent.isMovementStopped = true;
        Alita.Call.agent.isRotationStopped = true;
    }

    public override void ProcessInput(KeyCode code)
    {
        //if (code == KeyCode.KEY_Q && Alita.Call.skillset.skQ.Use())
        //    Alita.Call.SwitchState(Alita.Call.StateSkill_1);
        //else if (code == KeyCode.KEY_SPACE && Alita.Call.skillset.skDash.Use())
        //    Alita.Call.SwitchState(Alita.Call.StateDash);
    }
}

class AWalking2Spot : AWalking
{
    public override void OnExecute()
    {
        if (Alita.Call.agent.HasArrived)
            Alita.Call.SwitchState(Alita.Call.StateIdle);
    }

    public override void ProcessRaycast(RaycastHit hit)
    {
        if (hit.gameObject.GetLayer() == "Terrain")
        {
            Alita.Call.agent.SetDestination(hit.point);
        }
        else if (hit.gameObject.GetLayer() == "Enemy")
        {
            Alita.Call.currentTarget = hit.gameObject;
            Alita.Call.agent.SetDestination(hit.gameObject.transform.position);
            Alita.Call.SwitchState(Alita.Call.StateWalking2Enemy, false, false);
        }
    }
}

class AWalking2Enemy : AWalking
{
    public override void OnExecute()
    {
        if ((Alita.Call.transform.position -
             Alita.Call.currentTarget.transform.position)
             .magnitude < Alita.Call.ConstHitRadius)
        {
            Alita.Call.SwitchState(Alita.Call.StateAttacking);
        }
    }

    public override void ProcessRaycast(RaycastHit hit)
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
}

#endregion