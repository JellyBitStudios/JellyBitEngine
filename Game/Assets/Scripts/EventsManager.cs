using JellyBitEngine;
using System;
using System.Collections.Generic;

// This class is not tested, but it should work.

public enum Event_Type { None = 0, EnemyDead }

// This struct can be overloaded for future events
public class Event
{
    public Event_Type type;
}

public class EnemyDead_Event : Event
{
    public GameObject reference;
}

public struct Listener
{
    public string name;
    public object script;
    public string listener;
}

public sealed class EventsManager : JellyScript
{
    private static EventsManager instance;

    Queue<Event> eventsQueue = new Queue<Event>();
    List<Listener> listeners = new List<Listener>();

    public EventsManager()
    {
        instance = this;
    }

    public static EventsManager Call
    {
        get
        {
            return instance;
        }
    }

    public override void PreUpdate()
    {
        if (instance.eventsQueue.Count > 0)
        {
            Event newEvent = instance.eventsQueue.Dequeue();
            foreach(Listener listener in instance.listeners)
                listener.script.GetType().GetMethod(listener.listener).Invoke(listener.script, new object[] { newEvent });
        }
    }

    public override void OnStop()
    {
        instance.listeners = null;
    }

    public bool StartListening(string name, object script, string listener)
    {
        foreach (Listener listnr in listeners)
        {
            Debug.Log(listnr.name);
        }

        Listener listenerInstance = new Listener();

        listenerInstance.name = name;
        listenerInstance.script = script;
        listenerInstance.listener = listener;
        if (!listeners.Contains(listenerInstance))
        {
            Debug.Log("LISTENER PUSHED: from script " + script.GetType().ToString() + " with listener " + listener);
            listeners.Add(listenerInstance);
            return true;
        }
        Debug.Log("LISTENER NOT PUSHED: " +  "with name " + name + " from script " + script.GetType().ToString() + " with listener " + listener);
        return false;
    }

    public void StopListening(string name)
    {
        Listener list = listeners.Find(item => item.name == name);
        listeners.Remove(list);
    }

    public void PushEvent(Event newEvent)
    {
        Debug.Log("event pushed");
        eventsQueue.Enqueue(newEvent);
    }
}



