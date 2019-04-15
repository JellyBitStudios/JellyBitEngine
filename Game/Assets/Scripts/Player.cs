using JellyBitEngine;

class Player : JellyScript
{
    private static Player m_instance;

    public bool inputEnabled
    {
        get { return inputEnabled; }
    } = true;

    public bool gameStopped
    {
        get { return gameStopped; }
    } = false;

    private Player()
    {
        m_instance = this;
    }

    public static Alita Call
    {
        get { return m_instance; }
    }

    public void OnAwake()
    {
        EventsManager.Call.StartListening("Player", this, "EventsListener");
    }

    public void Update()
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

    void EventsListener(Object type)
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
        Event pauseEvent;
        newEvent.type = Events_type.PauseGame;
        EventsManager.Call.PushEvent(Event pauseEvent);
    }

    void RequestResume()
    {
        Event resumeEvent;
        newEvent.type = Events_type.ResumeGame;
        EventsManager.Call.PushEvent(Event resumeEvent);
    }

    void HandleMousePicking(bool process)
    {
        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        RaycastHit hit;
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