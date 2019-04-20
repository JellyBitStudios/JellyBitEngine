using System.Collections;
using System;
using JellyBitEngine;

public class CyborgMeleeCharacter : Character
{
    // GoToGameObject
    /// GoToAttackDistance
    public float attackDistance = 1.0f;
    /// GoToDangerDistance
    public float dangerDistance = 2.0f;

    // Attack
    public float attackRate = 10.0f;
    public float attackRateFluctuation = 0.0f;
    public float trackMaxAngularAcceleration = 90.0f;

    // Wander
    /// Strafe
    public float strafeMinTime = 1.0f;
    public float strafeMaxTime = 3.0f;
}

public class CyborgMeleeController : JellyScript
{
    #region PUBLIC_VARIABLES
    /// <Temporal>
    public LayerMask raycastMask = new LayerMask();
    /// </Temporal>

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
                //fsm.ChangeState(new Die());
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
        //fsm.ChangeState(new Wander(StateType.Wander));  
        fsm.ChangeState(new CM_GoToGameObject(CM_GoToGameObject.GoToGameObjectType.GoToDangerDistance));

        EventsManager.Call.StartListening("CyborgMelee", this, "OnEvent");

        // Agent
        agent.agentData.Radius = 1.0f;
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
            if (Physics.Raycast(ray, out hitInfo, float.MaxValue, raycastMask, SceneQueryFlags.Static))
            {
                agent.SetDestination(hitInfo.point);
                //wanderState = WanderStates.goToPosition;
            }
        }
    }
}