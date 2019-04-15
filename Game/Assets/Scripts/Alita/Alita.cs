using JellyBitEngine;

class Alita : JellyScript
{
    private static Alita m_instance;

    public AIdle StateIdle = new AIdle();
    public AWalking StateWalking = new AWalking();
    public AAttacking StateAttacking = new AAttacking();

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
            if (state != null)
                state.OnStop();
            state = value;
            state.OnStart();
        }
    }

    public AlitaCharacter character = new AlitaCharacter();
    public Agent agent;
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

    public override void Awake()
    {
        animator = gameObject.childs[0].GetComponent<Animator>();
        agent = gameObject.GetComponent<Agent>();
        StateIdle.OnAwake(this);
        StateWalking.OnAwake(this);
        StateAttacking.OnAwake(this);

        state = StateIdle;
        EventsManager.Call.StartListening("Alita", this, "EventsListener");
    }

    public override void Update()
    {
        if (!Player.Call.gameStopped)
            state.OnExecute();
    }

    public override void OnStop()
    {
        EventsManager.Call.StopListening("Alita");
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

