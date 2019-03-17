using System.Collections;
using JellyBitEngine;
using System;

public class AnimatorScript : JellyScript
{
    Animator anim = null;

    enum AnimStates
    {
        IDLE,
        RUNNING,
        DASH,
        ATTACK,
        SP_ATTACK
    }

    AnimStates animStates = AnimStates.IDLE;

    public override void Awake()
    {
        anim = gameObject.GetComponent<Animator>();
    }

    public void PlayIdle()
    {
        if (anim == null)
            anim = gameObject.GetComponent<Animator>();

        anim.PlayAnimation("idle_alita_anim");

        animStates = AnimStates.IDLE;
    }

    public void PlayRunning()
    {
        if (anim == null)
            anim = gameObject.GetComponent<Animator>();

        if(animStates != AnimStates.RUNNING)
            anim.PlayAnimation("run_alita_anim");

        animStates = AnimStates.RUNNING;
    }

    public void PlayAttack()
    {
        if (anim == null)
            anim = gameObject.GetComponent<Animator>();

        if (animStates != AnimStates.ATTACK)
            anim.PlayAnimation("attack_alita_anim");
        animStates = AnimStates.ATTACK;
    }

    public void PlayDash()
    {
        if (anim == null)
            anim = gameObject.GetComponent<Animator>();

        anim.PlayAnimation("alita_dash_anim");
        animStates = AnimStates.DASH;
    }

    public void PlayAreaAttack()
    {
        if (anim == null)
            anim = gameObject.GetComponent<Animator>();

        anim.PlayAnimation("special_attack_anim");
        animStates = AnimStates.SP_ATTACK;
    }

}