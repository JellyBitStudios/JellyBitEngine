using JellyBitEngine;
using System;

public class AlitaController : JellyScript
{
    #region PUBLIC_VARS
    // public in order to show in inspector
    public LayerMask        raycastLayer = new LayerMask(); // ADD ENEMY LAYER AND TERRAIN LAYER // SOMEWAY TO HARDCODE IT?

    [HideInInspector]
    public bool             inputEnabled = true; // USED TO CHECK IF THE PLAYER HAS CONTROL OVER ALITA OR NOT EX: Inventory opened?, Dialogs, etc
    #endregion

    #region PRIVATE_VARS
    // public in order to show in inspector
    AlitaCharacter          m_character = new AlitaCharacter();
    NavMeshAgent            m_agent;
    Animator                m_animator;
    GameObject              m_currentTarget
    {
        set
        {
            m_currentTarget = value;
            m_currentTargetMaterial = value.GetComponent<Material>();
        }

        get
        {
            return m_currentTarget;
        }
    }
    Material                m_currentTargetMaterial;

    enum AlitaStates        { Idle, Walking, WalkingToEnemy, Attacking };
    enum PickingStates      { None = -1, Terrain = 1, Enemy = 2 };
    enum CurrentAttack      { FirstAttack, SecondAttack, ThirdAttack };
    AlitaStates             m_state = AlitaStates.Idle;
    CurrentAttack           m_currentAttack = CurrentAttack.FirstAttack;
    PickingStates           m_lastPick = PickingStates.None;
    #endregion

    // ONCE THIS IS DONE -> ADD FX -> COMBO ATTACKS -> TARGET OUTLINE? -> DASH -> SKILLS -> DEAD

    public override void Awake()
    {
        m_agent = gameObject.GetComponent<NavMeshAgent>();
        m_animator = gameObject.childs[0].GetComponent<Animator>();
        UseIdle();
        //InitAlita();
        EventsManager.Call.StartListening("Target Dead", this, "Listening");
    }

    public void Listening(object type)
    {
        Event listenedEvent = (Event)type;
        switch (listenedEvent.type)
        {
            case Event_Type.EnemyDead:
                EnemyDead_Event EDEvent = (EnemyDead_Event)listenedEvent;
                if (EDEvent.reference == null)
                {
                    Debug.LogError("Invalid Reference: Event " + EDEvent.type.ToString() + " At " + ToString());
                    return;
                }
                m_currentTarget = null;
                UseIdle();
                break;
        }
    }

    public void InitAlita()
    {
        m_agent.maxAcceleration = m_character.accSpeed;
        m_agent.maxSpeed = m_character.velSpeed;
        UseIdle();
    }

    public override void Update()
    {
        // HERE WE HANDLE MOUSE INPUT AND SET DESTINATIONS
        if (Input.GetMouseButton(MouseKeyCode.MOUSE_RIGHT) && inputEnabled)
        {
            m_lastPick = HandleMousePicking();
            if (m_lastPick == PickingStates.Terrain && m_state != AlitaStates.Walking)
                UseWalking();
            else if (m_lastPick == PickingStates.Enemy && m_state != AlitaStates.WalkingToEnemy)
                UseWalkingToEnemy();
        }

        // HERE WE HANDLE ALITA STATES
        switch (m_state)
        {
            case AlitaStates.Walking:
                if (!m_agent.isWalking())
                    UseIdle();
                break;
            case AlitaStates.WalkingToEnemy:
                if (!m_agent.isWalking())
                    UseAttacking(m_currentAttack);
                break;
            case AlitaStates.Attacking:
                // TODO: ROTATE ALITA TO FOCUS TARGET USING currentTarget
                Vector3 dir = transform.position - m_currentTarget.transform.position;
                double angle = Math.Atan2(transform.forward.magnitude, dir.magnitude);
                // if (angle > threshold?)
                transform.rotation = Quaternion.Rotate(transform.up, (float)angle * Time.deltaTime * AlitaCharacter.attackRotConst) * transform.rotation;

                if (m_animator.AnimationFinished())
                    UseAttacking(m_currentAttack);

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
            string layer = hit.gameObject.GetLayer();
            if (layer == "Terrain")
            {
                if (m_lastPick == PickingStates.Enemy) { }
                    // TODO: m_currentTargetMaterial.SetResource("Red Outline");
                m_agent.SetDestination(hit.point);
                return PickingStates.Terrain;
            }
            else // in case of enemy. If target options increase add layer comparasion with "Enemy" layer
            {
                if (m_lastPick == PickingStates.Terrain) { }
                    // TODO: m_currentTargetMaterial.SetResource("Black Outline");
                m_currentTarget = hit.gameObject;
                Vector3 dir = (m_currentTarget.transform.position - transform.position).normalized();
                dir *= AlitaCharacter.attackRadiusConst;
                m_agent.SetDestination(m_currentTarget.transform.position + dir);

                return PickingStates.Enemy;
            }
        }
        return PickingStates.None;
    }

    void UseIdle()
    {
        m_state = AlitaStates.Idle;
        m_animator.PlayAnimation("idle_alita_anim");
        //Debug.Log("Stop");
    }

    void UseWalking()
    {
        Event newEvent = new Event();
        newEvent.type = Event_Type.None;
        EventsManager.Call.PushEvent(newEvent);
        m_state = AlitaStates.Walking;
        m_animator.PlayAnimation("anim_run_alita_fist");
        //Debug.Log("Walking");
    }

    void UseWalkingToEnemy()
    {
        m_state = AlitaStates.WalkingToEnemy;
        m_animator.PlayAnimation("anim_run_alita_fist");
        //Debug.Log("Walking to enemy");
    }

    void UseAttacking(CurrentAttack current)
    {
        m_state = AlitaStates.Attacking;
        if (current == CurrentAttack.FirstAttack)
        {
            m_animator.PlayAnimation("secondAttack_animation_alita");
            current = CurrentAttack.SecondAttack;
        }
        else if (current == CurrentAttack.SecondAttack)
        {
            m_animator.PlayAnimation("thirdAttack_animation_alita");
            current = CurrentAttack.ThirdAttack;
        }
        else
        {
            m_animator.PlayAnimation("anim_basic_attack_alita_fist");
            current = CurrentAttack.FirstAttack;
        }
        //Debug.Log("Fightning");
    }
}
