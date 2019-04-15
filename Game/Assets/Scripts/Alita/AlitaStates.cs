using JellyBitEngine;

public interface AState
{
    void OnAwake(Object controller); // Called at the beginning of the game
    void OnStart(); // Called once at the beginning of the frame
    void OnExecute(); // Called each frame
    void OnStop();  // Called once at the end of the state
    void OnPause(); // Called when the game or character must be paused
    void OnResume(); // Called to resume the game. OnPause previously called
    void ProcessInput(RaycastHit hit); // Process input
    void ProcessEvent(Event latestEvent); // called when a Alita recieves a new Event

    void NewTarget(GameObject target);
}

class AIdle : AState
{
    public void OnAwake(Object controller)
    {
    }

    void AState.OnStart()
    {
        Alita.Call.animator.PlayAnimation("idle_alita_anim");
    }

    void AState.OnExecute()
    {
    }

    void AState.OnStop()
    {
    }

    public void OnPause()
    {
    }

    public void OnResume()
    {
    }

    public void ProcessInput(RaycastHit hit)
    {
        if (hit.gameObject.GetLayer() == "Terrain")
        {
            Alita.Call.agent.SetDestination(hit.point);
            Alita.Call.state = Alita.Call.StateWalking;
        }
    }

    public void ProcessEvent(Event latestEvent)
    {
    }

    public void NewTarget(GameObject target)
    {
    }
}

class AWalking : AState
{
    public void OnAwake(Object controller)
    {
    }

    void AState.OnStart()
    {
        Alita.Call.animator.PlayAnimation("anim_run_alita_fist");
    }

    void AState.OnExecute()
    {
        Debug.Log("Walking :)");
    }

    public void OnStop()
    {
    }

    public void OnPause()
    {
    }

    public void OnResume()
    {
    }

    public void ProcessInput(RaycastHit hit)
    {
    }

    public void ProcessEvent(Event latestEvent)
    {
    }

    public void NewTarget(GameObject target)
    {
    }
}

class AAttacking : AState
{
    enum Anim { first, second, third }
    Anim currentAnim;

    public void OnAwake(Object controller)
    {
    }

    void AState.OnStart()
    {
        Alita.Call.animator.PlayAnimation("anim_basic_attack_alita_fist");
        currentAnim = Anim.first;
    }

    void AState.OnExecute()
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

    public void OnStop()
    {
    }

    public void OnPause()
    {
    }

    public void OnResume()
    {
    }

    public void ProcessInput(RaycastHit hit)
    {
    }

    public void ProcessEvent(Event latestEvent)
    {
    }

    public void NewTarget(GameObject target)
    {
    }
}
