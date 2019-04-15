using JellyBitEngine;

class Alita : JellyScript
{
    private static Alita m_instance;

    private AIdle m_idleState = new AIdle();
    private AWalking m_walkingState = new AWalking();
    private AAttacking m_attackingState = new AAttacking();

    private Alita()
    {
        m_instance = this;
    }

    public static Alita Call
    {
        get { return m_instance; }
    }

    public AState state
    {
        get { return state; }
        set
        {
            state.OnStop();
            state = value;
            state.OnStart();
        }
    }

    public AlitaCharacter character = new AlitaCharacter();
    public Animator animator;

    GameObject currentTarget
    {
        get { return currentTarget; }
        set
        {
            currentTarget = value;
            state.NewTarget(value);
        }
    }

    void Awake()
    {
        animator = gameObject.childs[0].GetComponent<Animator>();
        m_idleState.OnAwake(this);
        m_walkingState.OnAwake(this);
        m_attackingState.OnAwake(this);

        state = m_idleState;
        EventsManager.Call.StartListening("Alita", this, "EventsListener");
    }

    public override void Update()
    {
        if (!Player.Call.gameStopped)
            state.OnExecute();
    }

    public void ProcessInput(RaycastHit hit)
    {
        state.ProcessInput(hit);
    }

    public void EventsListener(object type)
    {
        Event listenedEvent = (Event)type;
        switch (listenedEvent.type)
        {
            case Event_Type.PauseGame:
                state.OnPause();
                break;
            case Event_Type.ResumeGame:
                state.OnResume();
                break;
        }
    }
}

