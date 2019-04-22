using System.Collections;
using System;
using JellyBitEngine;

public class CyborgMelee_Entity : NPC_Entity
{
    public CyborgMelee_Entity()
    {
        name = "Cyborg Melee";
    }

    // -------------------------------------------------

    // GoToGameObject
    /// GoToAttackDistance
    public float attackDistance = 1.5f;
    /// GoToDangerDistance
    public float dangerDistance = 5.0f;

    // Attack
    public float attackRate = 10.0f;
    public float attackRateFluctuation = 1.0f;

    // Wander
    /// Strafe
    public float strafeMinTime = 3.0f;
    public float strafeMaxTime = 10.0f;

    // Stun
    /// StunForce
    public float stunTime = 0.5f;
}

public class CyborgMeleeController : Controller
{
    #region INSPECTOR_VARIABLES
    // Entity
    public uint tmp_maxLife = 100;

    // NPC_Entity
    public uint tmp_damage = 10;
    public float tmp_distanceToTarget = 2.0f;
    public bool tmp_isBoss = false;

    // CyborgMelee_Entity
    public float tmp_attackDistance = 1.5f;
    public float tmp_dangerDistance = 5.0f;

    public float tmp_attackRate = 10.0f;
    public float tmp_attackRateFluctuation = 1.0f;

    public float tmp_strafeMinTime = 3.0f;
    public float tmp_strafeMaxTime = 10.0f;

    public float tmp_stunTime = 0.5f;
    #endregion

    #region PUBLIC_VARIABLES
    public CyborgMeleeFSM fsm;
    public CyborgMelee_Entity cbg_Entity;

    public Agent agent;
    public LineOfSight sight;
    public Animator animator;

    // -----

    public int CurrentLife
    {
        get { return entity.currentLife; }
        set
        {
            entity.currentLife = value;
            if (entity.currentLife <= 0)
            {
                entity.currentLife = 0;
                fsm.ChangeState(new CM_Die());
            }

            Debug.Log(cbg_Entity.name + " " + "current life" + ": " + entity.currentLife);
        }
    }

    // -----

    [HideInInspector]
    public bool isStunned = false;
    [HideInInspector]
    public bool isBeingAttacked = false;

    public bool drawGizmosCyborgMelee = true;
    #endregion

    // ----------------------------------------------------------------------------------------------------

    public override void Awake()
    {
        fsm = new CyborgMeleeFSM(this);
        entity = cbg_Entity = new CyborgMelee_Entity();

        agent = gameObject.GetComponent<Agent>();
        sight = gameObject.childs[0].GetComponent<LineOfSight>();
        animator = gameObject.childs[1].GetComponent<Animator>();
    }

    public override void Start()
    {
        AssignInspectorVariables();

        // -------------------------------------------------

        // Character
        entity.currentLife = (int)entity.maxLife;

        // Agent
        agent.agentData.Radius = gameObject.GetComponent<CapsuleCollider>().radius;

        // -------------------------------------------------

        fsm.ChangeState(new CM_Think());

        EventsManager.Call.StartListening("CyborgMelee", this, "OnEvent");
    }

    public override void Update()
    {
        AssignInspectorVariables();

        // -------------------------------------------------

        fsm.UpdateState();

        // -------------------------------------------------

        isBeingAttacked = false;
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

        Alita.Call.battleCircle.RemoveSimultaneousAttacker(this);
        Alita.Call.battleCircle.RemoveAttacker(this);
    }

    // Update
    public override void Actuate(uint hpModifier, Entity.Action action)
    {
        switch (action)
        {
            case Entity.Action.selected:

                // Nothing to do here...

                break;

            case Entity.Action.hit:
            case Entity.Action.thirdHit:

                isBeingAttacked = true;

                // Stun basic
                if (!isStunned)
                    fsm.ChangeState(new CM_StunBasic());

                CurrentLife -= (int)hpModifier;

                break;

            case Entity.Action.skillQ:

                // Stun force (always!)
                fsm.ChangeState(new CM_StunForce());

                CurrentLife -= (int)hpModifier;

                break;

            case Entity.Action.skillW:

                CurrentLife -= (int)hpModifier;

                break;
        }
    }

    // PostUpdate
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

    // ----------------------------------------------------------------------------------------------------

    private void AssignInspectorVariables()
    {
        // Entity
        entity.maxLife = tmp_maxLife;

        // NPC_Entity
        entity.dmg = tmp_damage;
        entity.distanceToTarget = tmp_distanceToTarget;
        entity.isBoss = tmp_isBoss;

        // CyborgMelee_Entity
        cbg_Entity.attackDistance = tmp_attackDistance;
        cbg_Entity.dangerDistance = tmp_dangerDistance;

        cbg_Entity.attackRate = tmp_attackRate;
        cbg_Entity.attackRateFluctuation = tmp_attackRateFluctuation;

        cbg_Entity.strafeMinTime = tmp_strafeMinTime;
        cbg_Entity.strafeMaxTime = tmp_strafeMaxTime;

        cbg_Entity.stunTime = tmp_stunTime;
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