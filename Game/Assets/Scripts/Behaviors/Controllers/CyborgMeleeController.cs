using System.Collections;
using System;
using JellyBitEngine;

public class CyborgMeleeCharacter : Character
{
    public CyborgMeleeCharacter()
    {

    }

    // --------------------------------------------------

    public string name = "Cyborg Melee";

    // -----

    // GoToGameObject
    /// GoToAttackDistance
    public float attackDistance = 0.5f;
    /// GoToDangerDistance
    public float dangerDistance = 4.0f;

    // Attack
    public float attackRate = 10.0f;
    public float attackRateFluctuation = 0.0f;

    //public float trackMaxAngularAcceleration = 360.0f;
    //public float trackMaxAngularVelocity = 360.0f;

    // Wander
    /// Strafe
    public float strafeMinTime = 1.0f;
    public float strafeMaxTime = 3.0f;
}

public class CyborgMeleeController : JellyScript
{
    #region PUBLIC_VARIABLES
    public CyborgMeleeCharacter character = new CyborgMeleeCharacter();

    public CyborgMeleeFSM fsm;

    public Agent agent;
    public LineOfSight sight;
    public Animator animator;

    public int CurrentLife
    {
        get { return character.currentLife; }
        set
        {
            if (value < 0)
                animator.PlayAnimation("melee_hurt_cyborg_animation");

            character.currentLife = value;
            if (character.currentLife <= 0)
            {
                character.currentLife = 0;
                fsm.ChangeState(new CM_Die());
            }

            Debug.Log("Cyborg melee life: " + character.currentLife);
        }
    }

    public bool drawGizmosCyborgMelee = true;
    #endregion

    // ----------------------------------------------------------------------------------------------------

    public override void Awake()
    {
        fsm = new CyborgMeleeFSM(this);

        agent = gameObject.GetComponent<Agent>();
        sight = gameObject.childs[0].GetComponent<LineOfSight>();
        animator = gameObject.childs[2].GetComponent<Animator>();
    }

    public override void Start()
    {
        // Character
        character.currentLife = (int)character.maxLife;

        // Agent
        agent.agentData.Radius = 1.0f;

        // -----

        fsm.ChangeState(new CM_GoToDangerDistance());

        EventsManager.Call.StartListening("CyborgMelee", this, "OnEvent");
    }

    public void OnEvent(object type)
    {
        Event myEvent = (Event)type;
        switch (myEvent.type)
        {
            case Event_Type.EnemyDie:
                {
                    EnemyEvent myEnemyEvent = (EnemyEvent)myEvent;
                    if (myEnemyEvent.gameObject == gameObject)
                    {
                        Debug.Log("I AM DEAD x_x");
                        GameObject.Destroy(gameObject);
                    }
                }
                break;
        }
    }

    public override void FixedUpdate()
    {
        //HandleInput();
        fsm.UpdateState();
    }

    public override void OnDrawGizmos()
    {
        if (!drawGizmosCyborgMelee)
            return;

        fsm.DrawGizmos();
    }

    public override void OnStop()
    {
        EventsManager.Call.StopListening(this);

        Alita.Call.battleCircle.RemoveSimultaneousAttacker(gameObject);
        Alita.Call.battleCircle.RemoveAttacker(gameObject);
    }

    // ----------------------------------------------------------------------------------------------------

    private void HandleInput()
    {
        if (Input.GetMouseButtonDown(MouseKeyCode.MOUSE_LEFT))
        {
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
            RaycastHit hitInfo;
            if (Physics.Raycast(ray, out hitInfo, float.MaxValue, LayerMask.GetMask("Terrain"), SceneQueryFlags.Static))
            {
                agent.SetDestination(hitInfo.point);
                //wanderState = WanderStates.goToPosition;
            }
        }
    }
}