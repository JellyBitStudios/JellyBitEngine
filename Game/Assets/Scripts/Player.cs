using JellyBitEngine;

class Player : JellyScript
{
    private static Player m_instance;

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

    public void OnAwake()
    {
        EventsManager.Call.StartListening("Player", this, "EventsListener");
    }

    public override void Update()
    {
        if (!gameStopped)
        {
            if (Input.GetMouseButton(MouseKeyCode.MOUSE_RIGHT) && inputEnabled)
                HandleMousePicking(true);
            else
                HandleMousePicking(false);

            // if n button pressed open inventory/options/etc 
        }
    }

    void EventsListener(object type)
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

    void HandleMousePicking(bool process)
    {
        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        RaycastHit hit;
        LayerMask raycastLayer = new LayerMask();
        if (Physics.Raycast(ray, out hit, float.MaxValue, raycastLayer, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            if (process)
                Alita.Call.ProcessInput(hit);

            // mark enemy as red etc
            string layer = hit.gameObject.GetLayer();
            if (layer == "Terrain")
            {
              
            }
            else
            {
              
            }
        }
        return;
    }

}