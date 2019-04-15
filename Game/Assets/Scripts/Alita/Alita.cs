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

    private AState m_state;

    public AlitaCharacter character = new AlitaCharacter();
    public Agent agent;
    public Animator animator;

    GameObject currentTarget
    {
        get { return currentTarget; }
        set
        {
            currentTarget = value;
            m_state.NewTarget(value);
        }
    }

    public override void Awake()
    {
        animator = gameObject.childs[0].GetComponent<Animator>();
        agent = gameObject.GetComponent<Agent>();
        StateIdle.OnAwake(this);
        StateWalking.OnAwake(this);
        StateAttacking.OnAwake(this);

        m_state = StateIdle;
        EventsManager.Call.StartListening("Alita", this, "EventsListener");
    }

    public override void Update()
    {
        if (!Player.Call.gameStopped)
            m_state.OnExecute();
    }

    public override void OnStop()
    {
        EventsManager.Call.StopListening("Alita");
    }

    public void SwitchState(AState newState)
    {
        m_state.OnStop();
        m_state = newState;
        m_state.OnStart();
    }

    public void ProcessInput(RaycastHit hit)
    {
        m_state.ProcessInput(hit);
    }

    public void EventsListener(object type)
    {
        Event listenedEvent = (Event)type;
        switch (listenedEvent.type)
        {
            case Event_Type.PauseGame:
                m_state.OnPause();
                break;
            case Event_Type.ResumeGame:
                m_state.OnResume();
                break;
        }
    }
}

