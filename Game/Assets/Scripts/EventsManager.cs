using JellyBitEngine;
using System;
using System.Collections.Generic;

// This class is not tested, but it should work.

public enum Event_Type { None = -1 }

// This class can be overloaded for future events
public struct Event
{
    public Event_Type type;
}

public struct Listener
{
    public Event_Type events;
    public object script;
    public string listener;
}

public sealed class EventsManager : JellyScript
{
    private static readonly EventsManager instance = new EventsManager();

    List<Event> eventsQueue = new List<Event>();
    List<Listener> listeners = new List<Listener>();

    private EventsManager() {}

    public static EventsManager Call
    {
        get
        {
            return instance;
        }
    }

    public override void PreUpdate()
    {
        //eventsQueue.Reverse();
        foreach (Event newEvent in eventsQueue)
        {
            Debug.Log("Iterating Queue");
            foreach (Listener listener in listeners)
            {
                Debug.Log("Calling");
                listener.script.GetType().GetMethod(listener.listener).Invoke(listener.script, new object[] { newEvent });
            }
        }

        //eventsQueue.Clear();
    }

    public bool StartListening(Event_Type type, object script, string listener)
    {
        Listener listenerInstance = new Listener();
        listenerInstance.events = type;
        listenerInstance.script = script;
        listenerInstance.listener = listener;
        if (!listeners.Contains(listenerInstance))
        {
            Debug.Log("LISTENER PUSHED: from script " + script.GetType().ToString() + " with listener " + listener + " for event " + type.ToString());
            listeners.Add(listenerInstance);
            return true;
        }
        return false;
    }

    public void StopListening(Event_Type type, object script, string listener)
    {
        Listener listenerInstance = new Listener();
        listenerInstance.events = type;
        listenerInstance.script = script;
        listenerInstance.listener = listener;
        listeners.Remove(listenerInstance);
    }

    public void PushEvent(Event newEvent)
    {
        Debug.Log("event pushed");
        eventsQueue.Add(newEvent);
    }
}



