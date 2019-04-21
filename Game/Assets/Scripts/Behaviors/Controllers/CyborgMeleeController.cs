using System.Collections;
using System;
using JellyBitEngine;

public class CyborgMelee_Entity : NPC_Entity
{
    public CyborgMelee_Entity()
    {
        name = "Cyborg Melee";
        //DistanceToTarget = 3.0f?
    }

    // -------------------------------------------------

    // GoToGameObject
    /// GoToAttackDistance
    public float attackDistance = 1.5f;
    /// GoToDangerDistance
    public float dangerDistance = 5.0f;

    // Attack
    public float attackRate = 10.0f;
    public float attackRateFluctuation = 0.0f;

    //public float trackMaxAngularAcceleration = 360.0f;
    //public float trackMaxAngularVelocity = 360.0f;

    // Wander
    /// Strafe
    public float strafeMinTime = 3.0f;
    public float strafeMaxTime = 10.0f;
}

public class CyborgMeleeController : Controller
{
    #region PUBLIC_VARIABLES
    public CyborgMeleeFSM fsm;

    public CyborgMelee_Entity cbg_Entity;

    public Agent agent;
    public LineOfSight sight;
    public Animator animator;

    public int CurrentLife
    {
        get { return entity.currentLife; }
        set
        {
            if (value < 0)
                animator.PlayAnimation("melee_hurt_cyborg_animation");

            entity.currentLife = value;
            if (entity.currentLife <= 0)
            {
                entity.currentLife = 0;
                fsm.ChangeState(new CM_Die());
            }

            Debug.Log("Cyborg melee life: " + entity.currentLife);
        }
    }

    public bool drawGizmosCyborgMelee = true;
    #endregion

    // ----------------------------------------------------------------------------------------------------

    public override void Awake()
    {
        entity = cbg_Entity = new CyborgMelee_Entity();
        fsm = new CyborgMeleeFSM(this);

        agent = gameObject.GetComponent<Agent>();
        sight = gameObject.childs[0].GetComponent<LineOfSight>();
        animator = gameObject.childs[1].GetComponent<Animator>();
    }

    public override void Start()
    {
        // Character
        entity.currentLife = (int)entity.maxLife;

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



    public override void Actuate(uint hpModifier, GameObject originGO, Alita_Entity.Action action)
    {
        switch (action)
        {
            case Alita_Entity.Action.hit:
                entity.currentLife -= (int)hpModifier;
                // currentLife -= hpModifier;
                break;
            case Alita_Entity.Action.skillQ:
                // move character backwards-> originGo.transform.positin - gameobject.transform.position etc
                // current life -= hpModifier;
                break;
            case Alita_Entity.Action.skillW:
                // ?
                break;
                // healing, other skills etc
        }
    }
}