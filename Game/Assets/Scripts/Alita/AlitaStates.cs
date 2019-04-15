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
    void OnStart()
    {
        AlitaController.Call.animator.PlayAnimation("idle_alita_anim");
    }

    void OnExecute()
    {
        // moving that ass
    }

    void OnStop()
    {
        // whoops
    }
}

class AWalking : AState
{
    void OnStart()
    {
        AlitaController.Call.animator.PlayAnimation("anim_run_alita_fist");
    }

    void OnExecute()
    {
    }
}

class AAttacking : AState
{
    enum Anim { first, second, third }
    Anim currentAnim;

    void OnStart()
    {
        AlitaController.Call.animator.PlayAnimation("anim_basic_attack_alita_fist");
        currentAnim = Anim.first;
    }

    void OnExecute()
    {
        if (AlitaController.Call.animator.AnimationFinished())
        {
            if (currentAnim == Anim.first)
            {
                AlitaController.Call.animator.PlayAnimation("secondAttack_animation_alita");
                currentAnim = Anim.second;
            }
            else if (currentAnim == Anim.first)
            {
                AlitaController.Call.animator.PlayAnimation("thirdAttack_animation_alita");
                currentAnim = Anim.third;
            }
            else
            {
                AlitaController.Call.animator.PlayAnimation("anim_basic_attack_alita_fist");
                currentAnim = Anim.first;
            }
        }
    }
}
