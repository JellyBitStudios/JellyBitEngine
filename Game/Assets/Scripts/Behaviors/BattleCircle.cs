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
    private List<GameObject> attackers;
    private List<GameObject> simultaneousAttackers;
    #endregion

    public override void Awake()
    {
        attackers = new List<GameObject>();
        simultaneousAttackers = new List<GameObject>();
    }

    public override void OnDrawGizmos()
    {
        foreach (GameObject attacker in attackers)
        {
            if (attacker == null)
                return;

            if (simultaneousAttackers.Contains(attacker))
                Debug.DrawSphere(1.0f, Color.White, attacker.transform.position, Quaternion.identity, Vector3.one);
            else
                Debug.DrawSphere(1.0f, Color.Red, attacker.transform.position, Quaternion.identity, Vector3.one);
        }
    }

    // ----------------------------------------------------------------------------------------------------
    // Attackers
    // ----------------------------------------------------------------------------------------------------

    public bool AddAttacker(GameObject attacker)
    {
        if (!attackers.Contains(attacker)
            && maxAttackers > 0)
        {
            // Limit number of attackers
            if (attackers.Count < maxAttackers)
            {
                attackers.Add(attacker);
                //Debug.Log("New attacker ADDED. Total attackers: " + attackers.Count);
                return true;
            }
            // Prioritize the attacker that Alita is attacking
            else if (attacker == Alita.Call.currentTarget)
            {
                // 1. Remove the first attacker
                GameObject firstAttacker = attackers[0];
                attackers.Remove(firstAttacker);

                // 2. Insert the new attacker
                attackers.Add(attacker);
                //Debug.Log("New attacker ADDED. Total attackers: " + attackers.Count);
                return true;
            }
        }

        //Debug.Log("New attacker REJECTED. Total attackers: " + attackers.Count);
        return false;
    }

    public bool RemoveAttacker(GameObject attacker)
    {
        if (attackers.Remove(attacker))
        {
            //Debug.Log("Attacker REMOVED. Total attackers: " + attackers.Count);
            return true;
        }

        //Debug.Log("Attacker could NOT be REMOVED. Total attackers: " + attackers.Count);
        return false;
    }

    public bool AttackersContains(GameObject attacker)
    {
        return attackers.Contains(attacker);
    }

    // ----------------------------------------------------------------------------------------------------
    // Simultaneous attackers
    // ----------------------------------------------------------------------------------------------------

    public bool AddSimultaneousAttacker(GameObject attacker)
    {
        if (!simultaneousAttackers.Contains(attacker)
            && simultaneousAttackers.Count < maxSimultaneousAttackers
            && maxSimultaneousAttackers > 0)
        {
            simultaneousAttackers.Add(attacker);
            //Debug.Log("New simultaneous attacker ADDED. Total simultaneous attackers: " + simultaneousAttackers.Count);
            return true;
        }

        //Debug.Log("New simultaneous attacker REJECTED. Total simultaneous attackers: " + simultaneousAttackers.Count);
        return false;
    }

    public bool RemoveSimultaneousAttacker(GameObject attacker)
    {
        if (simultaneousAttackers.Remove(attacker))
        {
            //Debug.Log("Attacker simultaneous REMOVED. Total simultaneous attackers: " + simultaneousAttackers.Count);
            return true;
        }

        //Debug.Log("Attacker simultaneous could NOT be REMOVED. Total simultaneous attackers: " + simultaneousAttackers.Count);
        return false;
    }
}