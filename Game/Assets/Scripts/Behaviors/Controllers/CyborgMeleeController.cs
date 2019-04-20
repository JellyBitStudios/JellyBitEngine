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
    public float attackDistance = 2.0f;
    /// GoToDangerDistance
    public float dangerDistance = 5.0f;

    // Attack
    public float attackRate = 10.0f;
    public float attackRateFluctuation = 0.0f;

    public float trackMaxAngularAcceleration = 90.0f;
    public float trackMaxAngularVelocity = 90.0f;

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

    public int CurrentLife
    {
        get { return character.currentLife; }
        set
        {
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

    public override void Update()
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