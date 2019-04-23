using JellyBitEngine;

public class Controller : JellyScript
{
    // Is important to have a reference to the entity here and at the controller
    // inheritance class in order to get 2 levels of abstraction.
    public NPC_Entity entity;

    // maybe move agent/animator/fsm here?
    // start at subController can call base.Start()

    public virtual void Actuate(uint hpModifier, Entity.Action action) { }
}