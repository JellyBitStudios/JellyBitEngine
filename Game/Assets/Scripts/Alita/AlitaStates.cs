using JellyBitEngine;

public abstract class AState
{
   public abstract void OnAwake(Object controller); // Called at the beginning of the game
   public abstract void OnStart(); // Called once at the beginning of the frame
   public abstract void OnExecute(); // Called each frame
   public abstract void OnStop();  // Called once at the end of the state
   public abstract void OnPause(); // Called when the game or character must be paused
   public abstract void OnResume(); // Called to resume the game. OnPause previously called
   public abstract void ProcessInput(RaycastHit hit); // Process input
   public abstract void ProcessEvent(Event latestEvent); // called when a Alita recieves a new Event
   public abstract void NewTarget(GameObject target);
}

class AIdle : AState
{
    public override void OnAwake(Object controller)
    {
    }

    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("idle_alita_anim");
    }

    public override void OnExecute()
    {
    }

    public override void OnStop()
    {
    }

    public override void OnPause()
    {
    }

    public override void OnResume()
    {
    }

    public override void ProcessInput(RaycastHit hit)
    {
        if (hit.gameObject.GetLayer() == "Terrain")
        {
            Alita.Call.agent.SetDestination(hit.point);
            Alita.Call.SwitchState(Alita.Call.StateWalking);
        }
    }

    public override void ProcessEvent(Event latestEvent)
    {
    }

    public override void NewTarget(GameObject target)
    {
    }
}

class AWalking : AState
{
    public override void OnAwake(Object controller)
    {
    }

    public override void OnStart()
    {
        Alita.Call.animator.PlayAnimation("anim_run_alita_fist");
    }

    public override void OnExecute()
    {
    }

    public override void OnStop()
    {
    }

    public override void OnPause()
    {
    }

    public override void OnResume()
    {
    }

    public override void ProcessInput(RaycastHit hit)
    {
    }

    public override void ProcessEvent(Event latestEvent)
    {
    }

    public override void NewTarget(GameObject target)
    {
    }
}

class AAttacking : AState
{
    enum Anim { first, second, third }
    Anim currentAnim;

    public override void OnAwake(Object controller)
    {
    }

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

    public override void OnStop()
    {
    }

    public override void OnPause()
    {
    }

    public override void OnResume()
    {
    }

    public override void ProcessInput(RaycastHit hit)
    {
    }

    public override void ProcessEvent(Event latestEvent)
    {
    }

    public override void NewTarget(GameObject target)
    {
    }
}
