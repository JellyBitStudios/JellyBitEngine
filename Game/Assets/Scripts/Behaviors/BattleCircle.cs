using System.Collections;
using System;
using JellyBitEngine;
using System.Collections.Generic;

// https://gamedevelopment.tutsplus.com/tutorials/battle-circle-ai-let-your-player-feel-like-theyre-fighting-lots-of-enemies--gamedev-13535

/*
Using a computer model, researchers had each virtual “wolf” follow two rules:
(1) move towards the prey until a certain distance is reached, and
(2) when other wolves are close to the prey, move away from [the other wolves].
These rules cause the pack members to behave in a way that resembles real wolves, circling up around the animal,
and when the prey tries to make a break for it, one wolf sometimes circles around and sets up an ambush, no communication required.
*/

public class BattleCircle : JellyScript
{
    #region PUBLIC_VARIABLES
    public uint maxAttackers = 3;
    public uint maxSimultaneousAttackers = 2;
    #endregion

    #region PRIVATE_VARIABLES
    private List<Controller> attackers;
    private List<Controller> simultaneousAttackers;
    #endregion

    public override void Awake()
    {
        attackers = new List<Controller>();
        simultaneousAttackers = new List<Controller>();
    }

    public override void OnDrawGizmos()
    {
        foreach (Controller attacker in attackers)
        {
            if (attacker == null)
                return;

            if (simultaneousAttackers.Contains(attacker))
                Debug.DrawSphere(1.0f, Color.White, attacker.transform.position, Quaternion.identity, Vector3.one);
            else
                Debug.DrawSphere(1.0f, Color.Black, attacker.transform.position, Quaternion.identity, Vector3.one);
        }
    }

    // ----------------------------------------------------------------------------------------------------
    // Attackers
    // ----------------------------------------------------------------------------------------------------

    public bool AddAttacker(Controller attacker)
    {
        if (attackers.Contains(attacker))
            return true;
        else if (maxAttackers > 0)
        {
            // Limit number of attackers
            if (GetAttackersCount() < maxAttackers
                || attacker.entity.isBoss)
            {
                attackers.Add(attacker);
                //Debug.Log("New attacker ADDED. Total attackers: " + attackers.Count);
                return true;
            }
            // Prioritize the attacker that Alita is attacking
            else if (attacker.gameObject == Alita.Call.currentTarget)
            {
                // 1. Remove the first attacker
                RemoveFirstAttacker();

                // 2. Insert the new attacker
                attackers.Add(attacker);
                //Debug.Log("New attacker ADDED. Total attackers: " + attackers.Count);
                return true;
            }
        }

        //Debug.Log("New attacker REJECTED. Total attackers: " + attackers.Count);
        return false;
    }

    public bool RemoveAttacker(Controller attacker)
    {
        if (attackers.Remove(attacker))
        {
            //Debug.Log("Attacker REMOVED. Total attackers: " + attackers.Count);
            return true;
        }

        //Debug.Log("Attacker could NOT be REMOVED. Total attackers: " + attackers.Count);
        return false;
    }

    public void RemoveFirstAttacker()
    {
        Controller firstAttacker = null;
        foreach (Controller attacker in attackers)
        {
            if (attacker == null
                || attacker.entity.isBoss)
                continue;

            firstAttacker = attacker;
            break;
        }

        if (firstAttacker != null)
            attackers.Remove(firstAttacker);
    }

    public bool AttackersContains(Controller attacker)
    {
        return attackers.Contains(attacker);
    }

    public uint GetAttackersCount()
    {
        uint count = 0;
        foreach (Controller attacker in attackers)
        {
            if (attacker == null)
                continue;

            if (!attacker.entity.isBoss)
                ++count;
        }
        return count;
    }

    // ----------------------------------------------------------------------------------------------------
    // Simultaneous attackers
    // ----------------------------------------------------------------------------------------------------

    public bool AddSimultaneousAttacker(Controller attacker)
    {
        if (simultaneousAttackers.Contains(attacker))
            return true;
        else if (maxSimultaneousAttackers > 0)
        {
            // Limit number of attackers
            if (GetSimultaneousAttackersCount() < maxSimultaneousAttackers
                || attacker.entity.isBoss)
            {
                simultaneousAttackers.Add(attacker);
                //Debug.Log("New attacker ADDED. Total attackers: " + attackers.Count);
                return true;
            }
            // Prioritize the attacker that Alita is attacking
            else if (attacker.gameObject == Alita.Call.currentTarget)
            {
                // 1. Remove the first simultaneous attacker
                RemoveFirstSimultaneousAttacker();

                // 2. Insert the new attacker
                simultaneousAttackers.Add(attacker);
                //Debug.Log("New attacker ADDED. Total attackers: " + attackers.Count);
                return true;
            }
        }

        //Debug.Log("New simultaneous attacker REJECTED. Total simultaneous attackers: " + simultaneousAttackers.Count);
        return false;
    }

    public bool RemoveSimultaneousAttacker(Controller attacker)
    {
        if (simultaneousAttackers.Remove(attacker))
        {
            //Debug.Log("Attacker simultaneous REMOVED. Total simultaneous attackers: " + simultaneousAttackers.Count);
            return true;
        }

        //Debug.Log("Attacker simultaneous could NOT be REMOVED. Total simultaneous attackers: " + simultaneousAttackers.Count);
        return false;
    }

    public void RemoveFirstSimultaneousAttacker()
    {
        Controller firstAttacker = null;
        foreach (Controller attacker in simultaneousAttackers)
        {
            if (attacker == null
                || attacker.entity.isBoss)
                continue;

            firstAttacker = attacker;
            break;
        }

        if (firstAttacker != null)
            simultaneousAttackers.Remove(firstAttacker);
    }

    public bool SimultaneousAttackersContains(Controller attacker)
    {
        return simultaneousAttackers.Contains(attacker);
    }

    public uint GetSimultaneousAttackersCount()
    {
        uint count = 0;
        foreach (Controller attacker in simultaneousAttackers)
        {
            if (attacker == null)
                continue;

            if (!attacker.entity.isBoss)
                ++count;
        }
        return count;
    }
}