using System.Collections;
using System;
using JellyBitEngine;

public class CyborgRangedCharacter : Entity
{
    public CyborgRangedCharacter()
    {
        name = "Cyborg Ranged";
        actualGoToSideProbability = goToSideProbability;
        actualHitRate = hitRate;
    }

    // --------------------------------------------------
    
    public uint minLife = 30;

    // -----

    // GoToGameObject
    /// GoToAttackDistance
    public float attackDistance = 1.0f;

    // Wander
    public float wanderMinTime = 1.0f;
    public float wanderMaxTime = 3.0f;

    // Look around
    public uint lookAroundMinTimes = 2;
    public uint lookAroundMaxTimes = 4;
    public float lookAroundMinTime = 0.1f;
    public float lookAroundMaxTime = 2.0f;
    public float lookAroundMinAngle = 10.0f;
    public float lookAroundMaxAngle = 30.0f;

    // Attack
    public float ActualGoToSideProbability
    {
        get { return actualGoToSideProbability; }
        set
        {
            actualGoToSideProbability = value;

            if (actualGoToSideProbability < 0.0f)
                actualGoToSideProbability = 0.0f;
            else if (actualGoToSideProbability > goToSideProbability)
                actualGoToSideProbability = goToSideProbability;
        }
    }
    private float actualGoToSideProbability = 0.0f;
    public float goToSideProbability = 0.5f;
    public float goToSideProbabilityFluctuation = 0.05f;

    // Hit
    public float actualHitRate = 0.0f;
    public float hitRate = 10.0f;
    public float hitRateFluctuation = 1.0f;

    // Wait
    /// WaitHit
    public float waitHitMinTime = 1.0f;
    public float waitHitMaxTime = 3.0f;

    // Other
    public float trackMaxAngularAcceleration = 90.0f;
    public float trackMaxAngularVelocity = 45.0f;
}

public class CyborgRangedController : JellyScript
{
    #region PUBLIC_VARIABLES
    public CyborgRangedCharacter character = new CyborgRangedCharacter();

    public CyborgRangedFSM fsm;

    public Agent agent;
    public LineOfSight sight;

    // --------------------------------------------------

    public int CurrentLife
    {
        get { return character.currentLife; }
        set
        {
            character.currentLife = value;
            if (character.currentLife <= 0)
            {
                character.currentLife = 0;
                fsm.ChangeState(new CR_Die());
            }
            //else if (character.currentLife <= character.minLife)
            //fsm.ChangeState(new GoToGameObject(Alita.Call.gameObject, GoToGameObject.GoToGameObjectType.Runaway));

            Debug.Log("Cyborg ranged life: " + character.currentLife);
        }
    }

    public bool drawGizmosCyborgRanged = true;
    #endregion

    // ----------------------------------------------------------------------------------------------------

    public override void Awake()
    {
        fsm = new CyborgRangedFSM(this);

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

        fsm.ChangeState(new CR_Wander());

        EventsManager.Call.StartListening("CyborgRanged", this, "OnEvent");
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
        fsm.UpdateState();
    }

    public override void OnDrawGizmos()
    {
        if (!drawGizmosCyborgRanged)
            return;

        fsm.DrawGizmos();
    }

    public override void OnStop()
    {
        EventsManager.Call.StopListening(this);
    }
}