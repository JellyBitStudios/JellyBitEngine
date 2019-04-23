using JellyBitEngine;

class Player : JellyScript
{
    private static Player m_instance;

    public static RaycastHit lastRaycastHit;

    public LayerMask raycastLayer = new LayerMask();

    public bool inputEnabled = true;

    public bool gameStopped = false;

    private Player()
    {
        m_instance = this;
    }

    public static Player Call
    {
        get { return m_instance; }
    }

    public override void Awake()
    {
        Debug.ClearConsole();
        EventsManager.Call.StartListening("Player", this, "EventsListener");
    }

    public override void Update()
    {
        if (!gameStopped)
        {
            if (Input.GetMouseButton(MouseKeyCode.MOUSE_LEFT))
                HandleMousePicking(true, true);
            else if (Input.GetMouseButtonDown(MouseKeyCode.MOUSE_RIGHT))
                HandleMousePicking(true, false);
            else
                HandleMousePicking(false);

            if (Input.GetKeyUp(KeyCode.KEY_Q))
                Alita.Call.ProcessInput(KeyCode.KEY_Q);
            else if (Input.GetKeyUp(KeyCode.KEY_SPACE))
                Alita.Call.ProcessInput(KeyCode.KEY_SPACE);
            else if (Input.GetKeyUp(KeyCode.KEY_W))
                Alita.Call.ProcessInput(KeyCode.KEY_W);
            // if n button pressed open inventory/options/etc 
        }
    }

    public override void OnStop()
    {
        EventsManager.Call.StopListening(this);
    }

    public void EventsListener(object type)
    {
        Event listenedEvent = (Event)type;
        switch (listenedEvent.type)
        {
            case Event_Type.PauseGame:
                gameStopped = true;
                break;
            case Event_Type.ResumeGame:
                gameStopped = false;
                break;
        }
    }

    void RequestPause()
    {
        Event pauseEvent = new Event();
        pauseEvent.type = Event_Type.PauseGame;
        EventsManager.Call.PushEvent(pauseEvent);
    }

    void RequestResume()
    {
        Event resumeEvent = new Event();
        resumeEvent.type = Event_Type.ResumeGame;
        EventsManager.Call.PushEvent(resumeEvent);
    }

    void HandleMousePicking(bool process, bool leftClick = true)
    {
        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        RaycastHit hit;
        if (Physics.Raycast(ray, out hit, float.MaxValue, raycastLayer, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            if (process)
                Alita.Call.ProcessRaycast(hit, leftClick);
            lastRaycastHit = hit;

            //string layer = hit.gameObject.GetLayer();
        }
        return;
    }

}