using JellyBitEngine;

public class AlitaController : JellyScript
{
    // public in order to show in inspector
    public LayerMask        raycastLayer = new LayerMask(); // ADD ENEMY LAYER AND TERRAIN LAYER // SOMEWAY TO HARDCODE IT?

    // public in order to show in inspector
    public AlitaCharacter   stats;
    NavMeshAgent            agent;
    Animator                animator;
    GameObject              currentTarget;

    enum AlitaStates        { Idle, Walking, WalkingToEnemy, Attacking };
    enum PickingStates      { None = -1, Terrain = 1, Enemy = 2 };
    enum CurrentAttack      { FirstAttack, SecondAttack, ThirdAttack };
    AlitaStates             state = AlitaStates.Idle;
    CurrentAttack           currentAttack = CurrentAttack.FirstAttack;

    bool                    inputEnabled = true; // USED TO CHECK IF THE PLAYER HAS CONTROL OVER ALITA OR NOT EX: Inventory opened?, Dialogs, etc
    
    // ONCE THIS IS DONE -> ADD FX -> COMBO ATTACKS -> TARGET OUTLINE? -> DASH -> SKILLS -> DEAD

    public override void Awake()
    {
        agent = gameObject.GetComponent<NavMeshAgent>();
        //animator = gameObject.GetComponent<Animator>();
        UseIdle();
        //InitAlita();
        EventsManager.Call.StartListening("Alita Listener", this, "Listening");
    }

    public void Listening(object type)
    {
        if (type.GetType().Equals(Event_Type.None))
            Debug.Log("Listened");
        else
            Debug.Log("Bad call");
    }

    public void InitAlita()
    {
        agent.maxAcceleration = stats.accSpeed;
        agent.maxSpeed = stats.velSpeed;
        UseIdle();
    }

    public override void Update()
    {
        // HERE WE HANDLE MOUSE INPUT AND SET DESTINATIONS
        if (Input.GetMouseButton(MouseKeyCode.MOUSE_RIGHT) && inputEnabled)
        {
            PickingStates returned = HandleMousePicking();
            if (returned == PickingStates.Terrain && state != AlitaStates.Walking)
                UseWalking();
            else if (returned == PickingStates.Enemy && state != AlitaStates.WalkingToEnemy)
                UseWalkingToEnemy();
        }

        // HERE WE HANDLE ALITA STATES
        switch (state)
        {
            case AlitaStates.Walking:
                if (!agent.isWalking())
                    UseIdle();
                break;
            case AlitaStates.WalkingToEnemy:
                if (!agent.isWalking())
                    UseAttacking(currentAttack);
                break;
            case AlitaStates.Attacking:
                // TODO: ROTATE ALITA TO FOCUS TARGET USING currentTarget
                // float angle = atan2(gameobject.transform.forward, currentTarget.transform.position);
                // gameobject.transform.rotation.rotateAxisAngle(alita.transform.position.up, angle * Time.deltaTime * stats.attackRotConst);

                // IF ANIM FINISHED
                //      IF LEFT CLICK IS RELEASE
                //          // USE IDLE
                //      ELSE
                //          // USE NEXT ATTACK

                // IF ENEMY IS DEAD
                //      currentTarget = null
                //      USE IDLE
                break;
        }
    }

    PickingStates HandleMousePicking()
    {
        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        RaycastHit hit;
        if (Physics.Raycast(ray, out hit, float.MaxValue, raycastLayer, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            if (hit.gameObject.GetLayer() == "Terrain")
            {
                agent.SetDestination(hit.point);
                return PickingStates.Terrain;
            }
            else // in case of enemy. If target options increase add layer comparasion with "Enemy" layer
            {
                currentTarget = hit.gameObject;
                Vector3 dir = (currentTarget.transform.position - transform.position).normalized();
                dir *= AlitaCharacter.attackRadiusConst;
                agent.SetDestination(currentTarget.transform.position + dir);
                return PickingStates.Enemy;
            }
        }
        return PickingStates.None;
    }

    void UseIdle()
    {
        state = AlitaStates.Idle;
        //animator.PlayAnimation("idle_animation_alita");
        Debug.Log("Stop");
    }

    void UseWalking()
    {
        Event newEvent = new Event();
        newEvent.type = Event_Type.None;
        EventsManager.Call.PushEvent(newEvent);
        state = AlitaStates.Walking;
        //animator.PlayAnimation("walk_animation_alita");
        Debug.Log("Walking");
    }

    void UseWalkingToEnemy()
    {
        state = AlitaStates.WalkingToEnemy;
       // animator.PlayAnimation("walk_animation_alita");
        Debug.Log("Walking to enemy");
    }

    void UseAttacking(CurrentAttack current)
    {
        state = AlitaStates.Attacking;
        if (current == CurrentAttack.FirstAttack)
        {
            //animator.PlayAnimation("secondAttack_animation_alita");
            current = CurrentAttack.SecondAttack;
        }
        else if (current == CurrentAttack.SecondAttack)
        {
            //animator.PlayAnimation("thirdAttack_animation_alita");
            current = CurrentAttack.ThirdAttack;
        }
        else
        {
            //animator.PlayAnimation("firstAttack_animation_alita");
            current = CurrentAttack.FirstAttack;
        }
        Debug.Log("Fightning");
    }
}
